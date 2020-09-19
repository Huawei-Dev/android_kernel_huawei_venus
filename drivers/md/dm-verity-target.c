/*
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * Author: Mikulas Patocka <mpatocka@redhat.com>
 *
 * Based on Chromium dm-verity driver (C) 2011 The Chromium OS Authors
 *
 * This file is released under the GPLv2.
 *
 * In the file "/sys/module/dm_verity/parameters/prefetch_cluster" you can set
 * default prefetch value. Data are read in "prefetch_cluster" chunks from the
 * hash device. Setting this greatly improves performance when data and hash
 * are on the same disk on different partitions on devices with poor random
 * access behavior.
 */

#include "dm-verity.h"
#include "dm-verity-fec.h"

#include <linux/module.h>
#include <linux/reboot.h>

#include <linux/mtd/hisi_nve_interface.h>
#include <linux/mtd/hisi_nve_number.h>
#include <linux/delay.h>

#if defined (CONFIG_HUAWEI_DSM)
#include <linux/jiffies.h>
#include <dsm/dsm_pub.h>
#include <linux/ctype.h>

static struct dsm_dev dm_dsm_dev = {
      .name = "dsm_dm_verity",
      .device_name = NULL,
      .ic_name = NULL,
      .module_name = NULL,
      .fops = NULL,
      .buff_size = 1024,
};

static unsigned long timeout;
#define DSM_REPORT_INTERVAL      (1)

static struct dsm_client *dm_dsm_dclient = NULL;

#define DM_VERITY_MAX_PRINT_ERRS	20
static unsigned long err_count;

#define HASH_ERR_VALUE		1
#endif

#define DM_MSG_PREFIX			"verity"

#define DM_VERITY_ENV_LENGTH		42
#define DM_VERITY_ENV_VAR_NAME		"DM_VERITY_ERR_BLOCK_NR"

#define DM_VERITY_IO_VEC_INLINE		16
#define DM_VERITY_MEMPOOL_SIZE		4
#define DM_VERITY_DEFAULT_PREFETCH_SIZE	262144

#define DM_VERITY_MAX_CORRUPTED_ERRS	100

#define DM_VERITY_OPT_LOGGING		"ignore_corruption"
#define DM_VERITY_OPT_RESTART		"restart_on_corruption"
#define DM_VERITY_OPT_IGN_ZEROES	"ignore_zero_blocks"

#define DM_VERITY_OPTS_MAX		(2 + DM_VERITY_OPTS_FEC)

static unsigned dm_verity_prefetch_cluster = DM_VERITY_DEFAULT_PREFETCH_SIZE;

module_param_named(prefetch_cluster, dm_verity_prefetch_cluster, uint, S_IRUGO | S_IWUSR);

struct dm_verity_prefetch_work {
	struct work_struct work;
	struct dm_verity *v;
	sector_t block;
	unsigned n_blocks;
};

/*
 * Auxiliary structure appended to each dm-bufio buffer. If the value
 * hash_verified is nonzero, hash of the block has been verified.
 *
 * The variable hash_verified is set to 0 when allocating the buffer, then
 * it can be changed to 1 and it is never reset to 0 again.
 *
 * There is no lock around this value, a race condition can at worst cause
 * that multiple processes verify the hash of the same buffer simultaneously
 * and write 1 to hash_verified simultaneously.
 * This condition is harmless, so we don't need locking.
 */
struct buffer_aux {
	int hash_verified;
};

/*
 * Initialize struct buffer_aux for a freshly created buffer.
 */
static void dm_bufio_alloc_callback(struct dm_buffer *buf)
{
	struct buffer_aux *aux = dm_bufio_get_aux_data(buf);

	aux->hash_verified = 0;
}

/*
 * Translate input sector number to the sector number on the target device.
 */
static sector_t verity_map_sector(struct dm_verity *v, sector_t bi_sector)
{
	return v->data_start + dm_target_offset(v->ti, bi_sector);
}

/*
 * Return hash position of a specified block at a specified tree level
 * (0 is the lowest level).
 * The lowest "hash_per_block_bits"-bits of the result denote hash position
 * inside a hash block. The remaining bits denote location of the hash block.
 */
static sector_t verity_position_at_level(struct dm_verity *v, sector_t block,
					 int level)
{
	return block >> (level * v->hash_per_block_bits);
}

/*
 * Wrapper for crypto_shash_init, which handles verity salting.
 */
static int verity_hash_init(struct dm_verity *v, struct shash_desc *desc, u32 count)
{
	int r;
	if (count)
		desc->tfm = v->tfm_sha256;
	else
		desc->tfm = v->tfm_sha2ce;
	desc->flags = CRYPTO_TFM_REQ_MAY_SLEEP;

	r = crypto_shash_init(desc);

	if (unlikely(r < 0)) {
		DMERR("crypto_shash_init failed: %d", r);
		return r;
	}

	if (likely(v->version >= 1)) {
		r = crypto_shash_update(desc, v->salt, v->salt_size);

		if (unlikely(r < 0)) {
			DMERR("crypto_shash_update failed: %d", r);
			return r;
		}
	}

	return 0;
}

static int verity_hash_update(struct dm_verity *v, struct shash_desc *desc,
			      const u8 *data, size_t len)
{
	int r = crypto_shash_update(desc, data, len);

	if (unlikely(r < 0))
		DMERR("crypto_shash_update failed: %d", r);

	return r;
}

static int verity_hash_final(struct dm_verity *v, struct shash_desc *desc,
			     u8 *digest)
{
	int r;

	if (unlikely(!v->version)) {
		r = crypto_shash_update(desc, v->salt, v->salt_size);

		if (r < 0) {
			DMERR("crypto_shash_update failed: %d", r);
			return r;
		}
	}

	r = crypto_shash_final(desc, digest);

	if (unlikely(r < 0))
		DMERR("crypto_shash_final failed: %d", r);

	return r;
}

int verity_hash_sel_sha(struct dm_verity *v, struct shash_desc *desc,
		const u8 *data, size_t len, u8 *digest, u32 count)
{
	int r;

	r = verity_hash_init(v, desc,count);
	if (unlikely(r < 0))
		return r;

	r = verity_hash_update(v, desc, data, len);
	if (unlikely(r < 0))
		return r;

	return verity_hash_final(v, desc, digest);
}


int verity_hash(struct dm_verity *v, struct shash_desc *desc,
		const u8 *data, size_t len, u8 *digest)
{
	int r;

	r = verity_hash_init(v, desc, 0);
	if (unlikely(r < 0))//lint !e730
		return r;

	r = verity_hash_update(v, desc, data, len);
	if (unlikely(r < 0))//lint !e730
		return r;

	return verity_hash_final(v, desc, digest);
}

static void verity_hash_at_level(struct dm_verity *v, sector_t block, int level,
				 sector_t *hash_block, unsigned *offset)
{
	sector_t position = verity_position_at_level(v, block, level);
	unsigned idx;

	*hash_block = v->hash_level_block[level] + (position >> v->hash_per_block_bits);

	if (!offset)
		return;

	idx = position & ((1 << v->hash_per_block_bits) - 1);
	if (!v->version)
		*offset = idx * v->digest_size;
	else
		*offset = idx << (v->hash_dev_block_bits - v->hash_per_block_bits);
}

#if defined (CONFIG_HUAWEI_DSM)
static void verity_dsm(struct dm_verity *v, enum verity_block_type type,
			     unsigned long long block, int error_no)
{
	const char *type_str = "";

	switch (type) {
	case DM_VERITY_BLOCK_TYPE_DATA:
		type_str = "data";
		break;
	case DM_VERITY_BLOCK_TYPE_METADATA:
		type_str = "metadata";
		break;
	default:
		BUG();
	}

	if (time_after(jiffies, timeout)) {
		if (!dsm_client_ocuppy(dm_dsm_dclient)) {
			dsm_client_record(dm_dsm_dclient, "%s: %s block %d is corrupted, dmd error num %d\n",
				v->data_dev->name, type_str, block, error_no);
			dsm_client_notify(dm_dsm_dclient, error_no);
		}

		timeout = jiffies + DSM_REPORT_INTERVAL*HZ;
	}
}

static void print_block_data(unsigned long long blocknr, unsigned char *data_to_dump
			, int start, int len)
{
	int i, j;
	int bh_offset = (start / 16) * 16;
	char row_data[17] = { 0, };
	char row_hex[50] = { 0, };
	char ch;

	if (err_count >= DM_VERITY_MAX_PRINT_ERRS)
		return;

	err_count++;

	printk(KERN_ERR " block error# : %llu, start offset(byte) : %d\n"
				, blocknr, start);
	printk(KERN_ERR "printing Hash dump %dbyte\n", len);
	printk(KERN_ERR "-------------------------------------------------\n");

	for (i = 0; i < (len + 15) / 16; i++) {
		for (j = 0; j < 16; j++) {
			ch = *(data_to_dump + bh_offset + j);
			if (start <= bh_offset + j
				&& start + len > bh_offset + j) {

				if (isascii(ch) && isprint(ch))
					sprintf(row_data + j, "%c", ch);
				else
					sprintf(row_data + j, ".");

				sprintf(row_hex + (j * 3), "%2.2x ", ch);
			} else {
				sprintf(row_data + j, " ");
				sprintf(row_hex + (j * 3), "-- ");
			}
		}

		printk(KERN_ERR "0x%4.4x : %s | %s\n"
				, bh_offset, row_hex, row_data);
		bh_offset += 16;
	}
	printk(KERN_ERR "---------------------------------------------------\n");
}
#endif

#if defined (CONFIG_DM_VERITY_HW_RETRY)

#define DM_MAX_ERR_COUNT		4

static int verity_write_nv(int value)
{
	struct hisi_nve_info_user nve;
	int ret;

	memset(&nve, 0, sizeof(nve));
	strncpy(nve.nv_name, "VMODE", strlen("VMODE")+1);
	nve.nv_number = NVE_VERIFY_MODE_NUM;
	nve.valid_size = 1;
	nve.nv_operation = NV_WRITE;
	nve.nv_data[0] = (unsigned char)value;
	ret = hisi_nve_direct_access(&nve);

	return ret;
}

static int verity_read_nv(void)
{
	struct hisi_nve_info_user nve;
	int ret;

	memset(&nve, 0, sizeof(nve));
	strncpy(nve.nv_name, "VMODE", strlen("VMODE")+1);
	nve.nv_number = NVE_VERIFY_MODE_NUM;
	nve.valid_size = 1;
	nve.nv_operation = NV_READ;

	ret = hisi_nve_direct_access(&nve);
	if (ret) {
		DMERR("read verify mode nve fail!");
		return -1;
	}

	return (int)nve.nv_data[0];
}


static int write_hw_hash_err_nv(int value)
{
	struct hisi_nve_info_user nve;
	int ret;

	memset(&nve, 0, sizeof(nve));
	strncpy(nve.nv_name, "HWHASH", strlen("HWHASH")+1);
	nve.nv_number = NVE_HW_HASH_ERR_NUM;
	nve.valid_size = 1;
	nve.nv_operation = NV_WRITE;
	nve.nv_data[0] = (unsigned char)value;
	ret = hisi_nve_direct_access(&nve);

	return ret;
}
#endif

/*
 * Handle verification errors.
 */
static int verity_handle_err(struct dm_verity *v, enum verity_block_type type,
			     unsigned long long block)
{
	char verity_env[DM_VERITY_ENV_LENGTH];
	char *envp[] = { verity_env, NULL };
	const char *type_str = "";
	int ret = 1;
	struct mapped_device *md = dm_table_get_md(v->ti->table);

	/* Corruption should be visible in device status in all modes */
	v->hash_failed = 1;

	if (v->corrupted_errs >= DM_VERITY_MAX_CORRUPTED_ERRS)
		return 1;

	v->corrupted_errs++;

	switch (type) {
	case DM_VERITY_BLOCK_TYPE_DATA:
		type_str = "data";
		break;
	case DM_VERITY_BLOCK_TYPE_METADATA:
		type_str = "metadata";
		break;
	default:
		BUG();
	}

	DMERR("%s: %s block %llu is corrupted", v->data_dev->name, type_str,
		block);

	if (v->corrupted_errs == DM_VERITY_MAX_CORRUPTED_ERRS)
		DMERR("%s: reached maximum errors", v->data_dev->name);

	snprintf(verity_env, DM_VERITY_ENV_LENGTH, "%s=%d,%llu",
		DM_VERITY_ENV_VAR_NAME, type, block);

	kobject_uevent_env(&disk_to_dev(dm_disk(md))->kobj, KOBJ_CHANGE, envp);

	if (v->mode == DM_VERITY_MODE_LOGGING)
		return 0;

	if (v->mode == DM_VERITY_MODE_RESTART) {
#if defined (CONFIG_DM_VERITY_HW_RETRY)
		int value = verity_read_nv();
		if (value < 0) {
			printk(KERN_ERR "read verify mode nve fail!");
			/* we need pay attention on this case */
			return 0;
		} else if (DM_MAX_ERR_COUNT == value)
			return 1;

		if (v->verify_failed_flag == 0) {
			if (value ++ >= DM_MAX_ERR_COUNT) {
				value = DM_MAX_ERR_COUNT;
			}
			if (verity_write_nv(value))
				printk(KERN_ERR "wirte verify mode nve fail!");

			v->verify_failed_flag = 1;
		}
#endif
		/*kernel_restart("dm-verity device corrupted");*/
		return 0;
	}

	return ret;
}

/*
 * Verify hash of a metadata block pertaining to the specified data block
 * ("block" argument) at a specified level ("level" argument).
 *
 * On successful return, verity_io_want_digest(v, io) contains the hash value
 * for a lower tree level or for the data block (if we're at the lowest level).
 *
 * If "skip_unverified" is true, unverified buffer is skipped and 1 is returned.
 * If "skip_unverified" is false, unverified buffer is hashed and verified
 * against current value of verity_io_want_digest(v, io).
 */
static int verity_verify_level(struct dm_verity *v, struct dm_verity_io *io,
			       sector_t block, int level, bool skip_unverified,
			       u8 *want_digest)
{
	struct dm_buffer *buf;
	struct buffer_aux *aux;
	u8 *data;
	int r;
	sector_t hash_block;
	unsigned offset;
	/* for retry */
	unsigned retry_count;




	verity_hash_at_level(v, block, level, &hash_block, &offset);

	data = dm_bufio_read(v->bufio, hash_block, &buf);
	if (unlikely(IS_ERR(data)))
		return (int)(PTR_ERR(data));

	aux = dm_bufio_get_aux_data(buf);

	if (!aux->hash_verified) 
	{
		if (skip_unverified) {
			r = 1;
			goto release_ret_r;
		}

		retry_count = 0;

LABEL:
		r = verity_hash_sel_sha(v, verity_io_hash_desc(v, io),
				data, 1 << v->hash_dev_block_bits,
				verity_io_real_digest(v, io), retry_count);
		if (unlikely(r < 0))
			goto release_ret_r;

		if (likely(memcmp(verity_io_real_digest(v, io), want_digest,
				  v->digest_size) == 0)) {
			aux->hash_verified = 1;
			if (retry_count != 0) {
			/* DSM-DMD INFO, soft hash OK while first ce hard hash fail */
#if defined (CONFIG_HUAWEI_DSM)
				verity_dsm(v, DM_VERITY_BLOCK_TYPE_METADATA, hash_block, DSM_DM_VERITY_CE_ERROR_NO);
				write_hw_hash_err_nv(HASH_ERR_VALUE);
#endif
				pr_err("[hash dm verity] CE hash fail,soft hash OK. retry_count = %d\n", retry_count);
			}
			/* else ce hash success */
		} else {
			/* hash fail */
			retry_count ++;
			if (retry_count == 1)
				goto LABEL;
			else if (verity_fec_decode(v, io, DM_VERITY_BLOCK_TYPE_METADATA,
					   hash_block, data, NULL) == 0) {
				/* fec success */
#if defined (CONFIG_HUAWEI_DSM)
				verity_dsm(v, DM_VERITY_BLOCK_TYPE_METADATA,
				hash_block, DSM_DM_VERITY_FEC_INFO_NO);
#endif
				aux->hash_verified = 1;
				pr_err("[hash dm verity] both ce and soft hash fail ,fec correct success. retry_count = %d\n", retry_count);
			} else if  (verity_handle_err(v,DM_VERITY_BLOCK_TYPE_METADATA,
					   hash_block)) {
				/* ce&soft hash fail, fec fail */
#if defined (CONFIG_HUAWEI_DSM)
				print_block_data((unsigned long long)hash_block,
					(unsigned char *)verity_io_real_digest(v, io),
					0, v->digest_size);
				print_block_data((unsigned long long)hash_block,
					(unsigned char *)want_digest,
					0, v->digest_size);
#endif
#if defined (CONFIG_HUAWEI_DSM)
				verity_dsm(v, DM_VERITY_BLOCK_TYPE_METADATA, hash_block, DSM_DM_VERITY_ERROR_NO);
#endif
				pr_err("[hash dm verity] both ce and soft hash fail ,fec fail. retry_count = %d\n", retry_count);
				r = -EIO;
				goto release_ret_r;
			} else {
				/* verity_handle_err success
				   Attention - Important: Do we need a dsm alarm?
				*/
				pr_err("[hash dm verity] verity_handle_err success\n");
			}
		} /* end hash fail */
	}
	data += offset;
	memcpy(want_digest, data, v->digest_size);
	r = 0;

release_ret_r:
	dm_bufio_release(buf);
	return r;
}

/*
 * Find a hash for a given block, write it to digest and verify the integrity
 * of the hash tree if necessary.
 */
int verity_hash_for_block(struct dm_verity *v, struct dm_verity_io *io,
			  sector_t block, u8 *digest, bool *is_zero)
{
	int r = 0, i;

	if (likely(v->levels)) {
		/*
		 * First, we try to get the requested hash for
		 * the current block. If the hash block itself is
		 * verified, zero is returned. If it isn't, this
		 * function returns 1 and we fall back to whole
		 * chain verification.
		 */
		r = verity_verify_level(v, io, block, 0, true, digest);
		if (likely(r <= 0))
			goto out;
	}

	memcpy(digest, v->root_digest, v->digest_size);

	for (i = v->levels - 1; i >= 0; i--) {
		r = verity_verify_level(v, io, block, i, false, digest);
		if (unlikely(r))
			goto out;
	}
out:
	if (!r && v->zero_digest)
		*is_zero = !memcmp(v->zero_digest, digest, v->digest_size);
	else
		*is_zero = false;

	return r;
}

/*
 * Calls function process for 1 << v->data_dev_block_bits bytes in the bio_vec
 * starting from iter.
 */
int verity_for_bv_block(struct dm_verity *v, struct dm_verity_io *io,
			struct bvec_iter *iter,
			int (*process)(struct dm_verity *v,
				       struct dm_verity_io *io, u8 *data,
				       size_t len))
{
	unsigned todo = 1 << v->data_dev_block_bits;
	struct bio *bio = dm_bio_from_per_bio_data(io, v->ti->per_bio_data_size);

	do {
		int r;
		u8 *page;
		unsigned len;
		struct bio_vec bv = bio_iter_iovec(bio, *iter);

		page = kmap_atomic(bv.bv_page);
		len = bv.bv_len;

		if (likely(len >= todo))
			len = todo;

		r = process(v, io, page + bv.bv_offset, len);
		kunmap_atomic(page);

		if (r < 0)
			return r;

		bio_advance_iter(bio, iter, len);
		todo -= len;
	} while (todo);

	return 0;
}

static int verity_bv_hash_update(struct dm_verity *v, struct dm_verity_io *io,
				 u8 *data, size_t len)
{
	return verity_hash_update(v, verity_io_hash_desc(v, io), data, len);
}

static int verity_bv_zero(struct dm_verity *v, struct dm_verity_io *io,
			  u8 *data, size_t len)
{
	memset(data, 0, len);
	return 0;
}

/*
 * Verify one "dm_verity_io" structure.
 */
static int verity_verify_io(struct dm_verity_io *io)
{
	bool is_zero;
	struct dm_verity *v = io->v;
	struct bvec_iter start;
	unsigned b;
	unsigned retry_count;

	/*  for retry */
	struct bvec_iter start2;
	struct bvec_iter start_retry;

	for (b = 0; b < io->n_blocks; b++) {
		int r;

		struct shash_desc *desc = verity_io_hash_desc(v, io);
		retry_count = 0;

LABEL:
		r = verity_hash_for_block(v, io, io->block + b,
					  verity_io_want_digest(v, io),
					  &is_zero);
		if (unlikely(r < 0))
			return r;

		if (is_zero) {
			/*
			 * If we expect a zero block, don't validate, just
			 * return zeros.
			 */
			r = verity_for_bv_block(v, io, &io->iter,
						verity_bv_zero);
			if (unlikely(r < 0))
				return r;

			continue;
		}

		r = verity_hash_init(v, desc, retry_count);
		if (unlikely(r < 0))
			return r;

		if (0 == retry_count) {
			start = io->iter;
			start_retry = start;		/*  for retry */
			r = verity_for_bv_block(v, io, &io->iter, verity_bv_hash_update);
		} else {
			/* retry */
			start = start_retry;
			start2 = start_retry;
			r = verity_for_bv_block(v, io, &start2, verity_bv_hash_update);
		}

		if (unlikely(r < 0))
			return r;

		r = verity_hash_final(v, desc, verity_io_real_digest(v, io));
		if (unlikely(r < 0))
			return r;

		if (likely(memcmp(verity_io_real_digest(v, io),
				  verity_io_want_digest(v, io), v->digest_size) == 0)) {
				if (retry_count != 0) {
					/* DSM-DMD INFO, soft hash OK while first ce hard hash fail */
#if defined (CONFIG_HUAWEI_DSM)
					verity_dsm(v, DM_VERITY_BLOCK_TYPE_DATA, io->block + b, DSM_DM_VERITY_CE_ERROR_NO);
					write_hw_hash_err_nv(HASH_ERR_VALUE);
#endif
					pr_err("CE hash fail,soft hash OK. retry_count = %d\n", retry_count);
				}
				/* else ce hash success */
		} else {
				/* hash fail */
				retry_count ++;
				if (retry_count == 1)
					goto LABEL;
				else if (verity_fec_decode(v, io, DM_VERITY_BLOCK_TYPE_DATA,
					   io->block + b, NULL, &start) == 0){
					/* fec success */
#if defined (CONFIG_HUAWEI_DSM)
					verity_dsm(v, DM_VERITY_BLOCK_TYPE_DATA,
						io->block + b, DSM_DM_VERITY_FEC_INFO_NO);
#endif
					pr_err("[hash dm verity data] both ce and soft hash fail ,fec correct success. retry_count = %d\n", retry_count);
				} else if (verity_handle_err(v, DM_VERITY_BLOCK_TYPE_DATA,
					   io->block + b)) {
				/* ce&soft hash fail, fec fail */
#if defined (CONFIG_HUAWEI_DSM)
				print_block_data((unsigned long long)(io->block+b),
					(unsigned char *)verity_io_real_digest(v, io),
					0, v->digest_size);
				print_block_data((unsigned long long)(io->block+b),
					(unsigned char *)verity_io_want_digest(v, io),
					0, v->digest_size);
#endif
#if defined (CONFIG_HUAWEI_DSM)
				verity_dsm(v, DM_VERITY_BLOCK_TYPE_DATA, io->block + b, DSM_DM_VERITY_ERROR_NO);
#endif
				pr_err("[hash dm verity data] both ce and soft hash fail ,fec fail. retry_count = %d\n", retry_count);
				return -EIO;
			} else {
				/* verity_handle_err success
				   Attention - Important: Do we need a dsm alarm?
				*/
				pr_err("[hash dm verity data] verity_handle_err success\n");
			}
		}

	}

	return 0;
}

/*
 * End one "io" structure with a given error.
 */
static void verity_finish_io(struct dm_verity_io *io, int error)
{
	struct dm_verity *v = io->v;
	struct bio *bio = dm_bio_from_per_bio_data(io, v->ti->per_bio_data_size);

	bio->bi_end_io = io->orig_bi_end_io;
	bio->bi_error = error;

	verity_fec_finish_io(io);

	bio_endio(bio);
}

static void verity_work(struct work_struct *w)
{
	struct dm_verity_io *io = container_of(w, struct dm_verity_io, work);

	verity_finish_io(io, verity_verify_io(io));
}

static void verity_end_io(struct bio *bio)
{
	struct dm_verity_io *io = bio->bi_private;

	if (bio->bi_error && !verity_fec_is_enabled(io->v)) {
		verity_finish_io(io, bio->bi_error);
		return;
	}

	INIT_WORK(&io->work, verity_work);
	queue_work(io->v->verify_wq, &io->work);
}

/*
 * Prefetch buffers for the specified io.
 * The root buffer is not prefetched, it is assumed that it will be cached
 * all the time.
 */
static void verity_prefetch_io(struct work_struct *work)
{
	struct dm_verity_prefetch_work *pw =
		container_of(work, struct dm_verity_prefetch_work, work);
	struct dm_verity *v = pw->v;
	int i;

	for (i = v->levels - 2; i >= 0; i--) {
		sector_t hash_block_start;
		sector_t hash_block_end;
		verity_hash_at_level(v, pw->block, i, &hash_block_start, NULL);
		verity_hash_at_level(v, pw->block + pw->n_blocks - 1, i, &hash_block_end, NULL);
		if (!i) {
			unsigned cluster = ACCESS_ONCE(dm_verity_prefetch_cluster);

			cluster >>= v->data_dev_block_bits;
			if (unlikely(!cluster))
				goto no_prefetch_cluster;

			if (unlikely(cluster & (cluster - 1)))
				cluster = 1 << __fls(cluster);

			hash_block_start &= ~(sector_t)(cluster - 1);
			hash_block_end |= cluster - 1;
			if (unlikely(hash_block_end >= v->hash_blocks))
				hash_block_end = v->hash_blocks - 1;
		}
no_prefetch_cluster:
		dm_bufio_prefetch(v->bufio, hash_block_start,
				  hash_block_end - hash_block_start + 1);
	}

	kfree(pw);
}

static void verity_submit_prefetch(struct dm_verity *v, struct dm_verity_io *io)
{
	struct dm_verity_prefetch_work *pw;

	pw = kmalloc(sizeof(struct dm_verity_prefetch_work),
		GFP_NOIO | __GFP_NORETRY | __GFP_NOMEMALLOC | __GFP_NOWARN);

	if (!pw)
		return;

	INIT_WORK(&pw->work, verity_prefetch_io);
	pw->v = v;
	pw->block = io->block;
	pw->n_blocks = io->n_blocks;
	queue_work(v->verify_wq, &pw->work);
}

/*
 * Bio map function. It allocates dm_verity_io structure and bio vector and
 * fills them. Then it issues prefetches and the I/O.
 */
int verity_map(struct dm_target *ti, struct bio *bio)
{
	struct dm_verity *v = ti->private;
	struct dm_verity_io *io;

	bio->bi_bdev = v->data_dev->bdev;
	bio->bi_iter.bi_sector = verity_map_sector(v, bio->bi_iter.bi_sector);

	if (((unsigned)bio->bi_iter.bi_sector | bio_sectors(bio)) &
	    ((1 << (v->data_dev_block_bits - SECTOR_SHIFT)) - 1)) {
		DMERR_LIMIT("unaligned io");
		return -EIO;
	}

	if (bio_end_sector(bio) >>
	    (v->data_dev_block_bits - SECTOR_SHIFT) > v->data_blocks) {
		DMERR_LIMIT("io out of range");
		return -EIO;
	}

	if (bio_data_dir(bio) == WRITE)
		return -EIO;

	io = dm_per_bio_data(bio, ti->per_bio_data_size);
	io->v = v;
	io->orig_bi_end_io = bio->bi_end_io;
	io->block = bio->bi_iter.bi_sector >> (v->data_dev_block_bits - SECTOR_SHIFT);
	io->n_blocks = bio->bi_iter.bi_size >> v->data_dev_block_bits;

	bio->bi_end_io = verity_end_io;
	bio->bi_private = io;
	io->iter = bio->bi_iter;

	verity_fec_init_io(io);

	verity_submit_prefetch(v, io);

	generic_make_request(bio);

	return DM_MAPIO_SUBMITTED;
}

/*
 * Status: V (valid) or C (corruption found)
 */
void verity_status(struct dm_target *ti, status_type_t type,
			  unsigned status_flags, char *result, unsigned maxlen)
{
	struct dm_verity *v = ti->private;
	unsigned args = 0;
	unsigned sz = 0;
	unsigned x;

	switch (type) {
	case STATUSTYPE_INFO:
		DMEMIT("%c", v->hash_failed ? 'C' : 'V');
		break;
	case STATUSTYPE_TABLE:
		DMEMIT("%u %s %s %u %u %llu %llu %s ",
			v->version,
			v->data_dev->name,
			v->hash_dev->name,
			1 << v->data_dev_block_bits,
			1 << v->hash_dev_block_bits,
			(unsigned long long)v->data_blocks,
			(unsigned long long)v->hash_start,
			v->alg_name_sha2ce
			);
		for (x = 0; x < v->digest_size; x++)
			DMEMIT("%02x", v->root_digest[x]);
		DMEMIT(" ");
		if (!v->salt_size)
			DMEMIT("-");
		else
			for (x = 0; x < v->salt_size; x++)
				DMEMIT("%02x", v->salt[x]);
		if (v->mode != DM_VERITY_MODE_EIO)
			args++;
		if (verity_fec_is_enabled(v))
			args += DM_VERITY_OPTS_FEC;
		if (v->zero_digest)
			args++;
		if (!args)
			return;
		DMEMIT(" %u", args);
		if (v->mode != DM_VERITY_MODE_EIO) {
			DMEMIT(" ");
			switch (v->mode) {
			case DM_VERITY_MODE_LOGGING:
				DMEMIT(DM_VERITY_OPT_LOGGING);
				break;
			case DM_VERITY_MODE_RESTART:
				DMEMIT(DM_VERITY_OPT_RESTART);
				break;
			default:
				BUG();
			}
		}
		if (v->zero_digest)
			DMEMIT(" " DM_VERITY_OPT_IGN_ZEROES);
		sz = verity_fec_status_table(v, sz, result, maxlen);
		break;
	}
}

static int verity_prepare_ioctl(struct dm_target *ti,
		struct block_device **bdev, fmode_t *mode)
{
	struct dm_verity *v = ti->private;
	
	*bdev = v->data_dev->bdev;

	if (v->data_start ||
	    ti->len != i_size_read(v->data_dev->bdev->bd_inode) >> SECTOR_SHIFT)
		return 1;

	return 0;
}

int verity_iterate_devices(struct dm_target *ti,
				  iterate_devices_callout_fn fn, void *data)
{
	struct dm_verity *v = ti->private;

	return fn(ti, v->data_dev, v->data_start, ti->len, data);
}

void verity_io_hints(struct dm_target *ti, struct queue_limits *limits)
{
	struct dm_verity *v = ti->private;

	if (limits->logical_block_size < 1 << v->data_dev_block_bits)
		limits->logical_block_size = 1 << v->data_dev_block_bits;

	if (limits->physical_block_size < 1 << v->data_dev_block_bits)
		limits->physical_block_size = 1 << v->data_dev_block_bits;

	blk_limits_io_min(limits, limits->logical_block_size);
}

void verity_dtr(struct dm_target *ti)
{
	struct dm_verity *v = ti->private;

	if (v->verify_wq)
		destroy_workqueue(v->verify_wq);

	if (v->vec_mempool)
		mempool_destroy(v->vec_mempool);

	if (v->bufio)
		dm_bufio_client_destroy(v->bufio);

	kfree(v->salt);
	kfree(v->root_digest);
	kfree(v->zero_digest);

	if (v->tfm_sha2ce)
		crypto_free_shash(v->tfm_sha2ce);
	if (v->tfm_sha256)
		crypto_free_shash(v->tfm_sha256);

	kfree(v->alg_name_sha2ce);
	kfree(v->alg_name_sha256);

	if (v->hash_dev)
		dm_put_device(ti, v->hash_dev);

	if (v->data_dev)
		dm_put_device(ti, v->data_dev);

	verity_fec_dtr(v);

	kfree(v);
}

static int verity_alloc_zero_digest(struct dm_verity *v)
{
	int r = -ENOMEM;
	struct shash_desc *desc;
	u8 *zero_data;

	v->zero_digest = kmalloc(v->digest_size, GFP_KERNEL);

	if (!v->zero_digest)
		return r;

	desc = kmalloc(v->shash_descsize, GFP_KERNEL);

	if (!desc)
		return r; /* verity_dtr will free zero_digest */

	zero_data = kzalloc(1 << v->data_dev_block_bits, GFP_KERNEL);

	if (!zero_data)
		goto out;

	r = verity_hash(v, desc, zero_data, 1 << v->data_dev_block_bits,
			v->zero_digest);

out:
	kfree(desc);
	kfree(zero_data);

	return r;
}

static int verity_parse_opt_args(struct dm_arg_set *as, struct dm_verity *v)
{
	int r;
	unsigned argc;
	struct dm_target *ti = v->ti;
	const char *arg_name;

	static struct dm_arg _args[] = {
		{0, DM_VERITY_OPTS_MAX, "Invalid number of feature args"},
	};

	r = dm_read_arg_group(_args, as, &argc, &ti->error);
	if (r)
		return -EINVAL;

	if (!argc)
		return 0;

	do {
		arg_name = dm_shift_arg(as);
		argc--;

		if (!strcasecmp(arg_name, DM_VERITY_OPT_LOGGING)) {
			v->mode = DM_VERITY_MODE_LOGGING;
			continue;

		} else if (!strcasecmp(arg_name, DM_VERITY_OPT_RESTART)) {
			v->mode = DM_VERITY_MODE_RESTART;
			continue;

		} else if (!strcasecmp(arg_name, DM_VERITY_OPT_IGN_ZEROES)) {
			r = verity_alloc_zero_digest(v);
			if (r) {
				ti->error = "Cannot allocate zero digest";
				return r;
			}
			continue;

		} else if (verity_is_fec_opt_arg(arg_name)) {
			r = verity_fec_parse_opt_args(as, v, &argc, arg_name);
			if (r)
				return r;
			continue;
		}

		ti->error = "Unrecognized verity feature request";
		return -EINVAL;
	} while (argc && !r);

	return r;
}

/*
 * Target parameters:
 *	<version>	The current format is version 1.
 *			Vsn 0 is compatible with original Chromium OS releases.
 *	<data device>
 *	<hash device>
 *	<data block size>
 *	<hash block size>
 *	<the number of data blocks>
 *	<hash start block>
 *	<algorithm>
 *	<digest>
 *	<salt>		Hex string or "-" if no salt.
 */
int verity_ctr(struct dm_target *ti, unsigned argc, char **argv)
{
	struct dm_verity *v;
	struct dm_arg_set as;
	unsigned int num;
	unsigned long long num_ll;
	int r;
	int i;
	sector_t hash_position;
	char dummy;

	v = kzalloc(sizeof(struct dm_verity), GFP_KERNEL);
	if (!v) {
		ti->error = "Cannot allocate verity structure";
		return -ENOMEM;
	}
	ti->private = v;
	v->ti = ti;

#if defined (CONFIG_DM_VERITY_HW_RETRY)
	v->verify_failed_flag = 0;
#endif

	r = verity_fec_ctr_alloc(v);
	if (r)
		goto bad;

	if ((dm_table_get_mode(ti->table) & ~FMODE_READ)) {
		ti->error = "Device must be readonly";
		r = -EINVAL;
		goto bad;
	}

	if (argc < 10) {
		ti->error = "Not enough arguments";
		r = -EINVAL;
		goto bad;
	}

	if (sscanf(argv[0], "%u%c", &num, &dummy) != 1 ||
	    num > 1) {
		ti->error = "Invalid version";
		r = -EINVAL;
		goto bad;
	}
	v->version = num;

	r = dm_get_device(ti, argv[1], FMODE_READ, &v->data_dev);
	if (r) {
		ti->error = "Data device lookup failed";
		goto bad;
	}

	r = dm_get_device(ti, argv[2], FMODE_READ, &v->hash_dev);
	if (r) {
		ti->error = "Data device lookup failed";
		goto bad;
	}

	if (sscanf(argv[3], "%u%c", &num, &dummy) != 1 ||
	    !num || (num & (num - 1)) ||
	    num < bdev_logical_block_size(v->data_dev->bdev) ||
	    num > PAGE_SIZE) {
		ti->error = "Invalid data device block size";
		r = -EINVAL;
		goto bad;
	}
	v->data_dev_block_bits = __ffs(num);

	if (sscanf(argv[4], "%u%c", &num, &dummy) != 1 ||
	    !num || (num & (num - 1)) ||
	    num < bdev_logical_block_size(v->hash_dev->bdev) ||
	    num > INT_MAX) {
		ti->error = "Invalid hash device block size";
		r = -EINVAL;
		goto bad;
	}
	v->hash_dev_block_bits = __ffs(num);

	if (sscanf(argv[5], "%llu%c", &num_ll, &dummy) != 1 ||
	    (sector_t)(num_ll << (v->data_dev_block_bits - SECTOR_SHIFT))
	    >> (v->data_dev_block_bits - SECTOR_SHIFT) != num_ll) {
		ti->error = "Invalid data blocks";
		r = -EINVAL;
		goto bad;
	}
	v->data_blocks = num_ll;

	if (ti->len > (v->data_blocks << (v->data_dev_block_bits - SECTOR_SHIFT))) {
		ti->error = "Data device is too small";
		r = -EINVAL;
		goto bad;
	}

	if (sscanf(argv[6], "%llu%c", &num_ll, &dummy) != 1 ||
	    (sector_t)(num_ll << (v->hash_dev_block_bits - SECTOR_SHIFT))
	    >> (v->hash_dev_block_bits - SECTOR_SHIFT) != num_ll) {
		ti->error = "Invalid hash start";
		r = -EINVAL;
		goto bad;
	}
	v->hash_start = num_ll;

	v->alg_name_sha2ce = kstrdup(argv[7], GFP_KERNEL);
	v->alg_name_sha256 = kstrdup(argv[7], GFP_KERNEL);
	if ((NULL == v->alg_name_sha2ce) || (NULL == v->alg_name_sha256)) {
		ti->error = "Cannot allocate algorithm name";
		r = -ENOMEM;
		goto bad;
	}

#if defined(CONFIG_DM_HISI_SHA_USE_SOFT)
	if (strncmp(v->alg_name_sha2ce, "sha256", strlen("sha256")+1) == 0) {
		strncpy(v->alg_name_sha2ce,"sha2ce", strlen("sha2ce")+1);
	}
#endif

	v->tfm_sha2ce = crypto_alloc_shash(v->alg_name_sha2ce, 0, 0);
	if (IS_ERR(v->tfm_sha2ce)) {
		ti->error = "Cannot initialize hash function";
		r = PTR_ERR(v->tfm_sha2ce);
		v->tfm_sha2ce = NULL;
		goto bad;
	}

	v->tfm_sha256 = crypto_alloc_shash(v->alg_name_sha256, 0, 0);
	if (IS_ERR(v->tfm_sha256)) {
		ti->error = "Cannot initialize hash function";
		r = (int)PTR_ERR(v->tfm_sha256);
		v->tfm_sha256 = NULL;
		goto bad;
	}

	v->digest_size = crypto_shash_digestsize(v->tfm_sha2ce);
	if ((1 << v->hash_dev_block_bits) < v->digest_size * 2) {
		ti->error = "Digest size too big";
		r = -EINVAL;
		goto bad;
	}
	v->shash_descsize =
		sizeof(struct shash_desc) + crypto_shash_descsize(v->tfm_sha2ce);

	v->root_digest = kmalloc(v->digest_size, GFP_KERNEL);
	if (!v->root_digest) {
		ti->error = "Cannot allocate root digest";
		r = -ENOMEM;
		goto bad;
	}
	if (strlen(argv[8]) != v->digest_size * 2 ||
	    hex2bin(v->root_digest, argv[8], v->digest_size)) {
		ti->error = "Invalid root digest";
		r = -EINVAL;
		goto bad;
	}

	if (strcmp(argv[9], "-")) {
		v->salt_size = strlen(argv[9]) / 2;
		v->salt = kmalloc(v->salt_size, GFP_KERNEL);
		if (!v->salt) {
			ti->error = "Cannot allocate salt";
			r = -ENOMEM;
			goto bad;
		}
		if (strlen(argv[9]) != v->salt_size * 2 ||
		    hex2bin(v->salt, argv[9], v->salt_size)) {
			ti->error = "Invalid salt";
			r = -EINVAL;
			goto bad;
		}
	}

	argv += 10;
	argc -= 10;

	/* Optional parameters */
	if (argc) {
		as.argc = argc;
		as.argv = argv;

		r = verity_parse_opt_args(&as, v);
		if (r < 0)
			goto bad;
	}

	v->hash_per_block_bits =
		__fls((1 << v->hash_dev_block_bits) / v->digest_size);

	v->levels = 0;
	if (v->data_blocks)
		while (v->hash_per_block_bits * v->levels < 64 &&
		       (unsigned long long)(v->data_blocks - 1) >>
		       (v->hash_per_block_bits * v->levels))
			v->levels++;

	if (v->levels > DM_VERITY_MAX_LEVELS) {
		ti->error = "Too many tree levels";
		r = -E2BIG;
		goto bad;
	}

	hash_position = v->hash_start;
	for (i = v->levels - 1; i >= 0; i--) {
		sector_t s;
		v->hash_level_block[i] = hash_position;
		s = (v->data_blocks + ((sector_t)1 << ((i + 1) * v->hash_per_block_bits)) - 1)
					>> ((i + 1) * v->hash_per_block_bits);
		if (hash_position + s < hash_position) {
			ti->error = "Hash device offset overflow";
			r = -E2BIG;
			goto bad;
		}
		hash_position += s;
	}
	v->hash_blocks = hash_position;

	v->bufio = dm_bufio_client_create(v->hash_dev->bdev,
		1 << v->hash_dev_block_bits, 1, sizeof(struct buffer_aux),
		dm_bufio_alloc_callback, NULL);
	if (IS_ERR(v->bufio)) {
		ti->error = "Cannot initialize dm-bufio";
		r = PTR_ERR(v->bufio);
		v->bufio = NULL;
		goto bad;
	}

	if (dm_bufio_get_device_size(v->bufio) < v->hash_blocks) {
		ti->error = "Hash device is too small";
		r = -E2BIG;
		goto bad;
	}

	ti->per_bio_data_size = roundup(sizeof(struct dm_verity_io) + v->shash_descsize + v->digest_size * 2, __alignof__(struct dm_verity_io));

	v->vec_mempool = mempool_create_kmalloc_pool(DM_VERITY_MEMPOOL_SIZE,
					BIO_MAX_PAGES * sizeof(struct bio_vec));
	if (!v->vec_mempool) {
		ti->error = "Cannot allocate vector mempool";
		r = -ENOMEM;
		goto bad;
	}

	/* WQ_UNBOUND greatly improves performance when running on ramdisk */
	v->verify_wq = alloc_workqueue("kverityd", WQ_CPU_INTENSIVE | WQ_MEM_RECLAIM | WQ_UNBOUND, num_online_cpus());
	if (!v->verify_wq) {
		ti->error = "Cannot allocate workqueue";
		r = -ENOMEM;
		goto bad;
	}

	ti->per_bio_data_size = sizeof(struct dm_verity_io) +
				v->shash_descsize + v->digest_size * 2;

	r = verity_fec_ctr(v);
	if (r)
		goto bad;

	ti->per_bio_data_size = roundup(ti->per_bio_data_size,
					__alignof__(struct dm_verity_io));

	return 0;

bad:
	verity_dtr(ti);

	return r;
}

static struct target_type verity_target = {
	.name		= "verity",
	.version	= {1, 3, 0},
	.module		= THIS_MODULE,
	.ctr		= verity_ctr,
	.dtr		= verity_dtr,
	.map		= verity_map,
	.status		= verity_status,
	.prepare_ioctl	= verity_prepare_ioctl,
	.iterate_devices = verity_iterate_devices,
	.io_hints	= verity_io_hints,
};

static int __init dm_verity_init(void)
{
	int r;

	r = dm_register_target(&verity_target);
	if (r < 0)
		DMERR("register failed %d", r);

#if defined (CONFIG_HUAWEI_DSM)
    if (!dm_dsm_dclient) {
        dm_dsm_dclient = dsm_register_client(&dm_dsm_dev);
		if (NULL == dm_dsm_dclient) {
			DMERR("[%s]dsm_register_client register fail.\n", __func__);
		}
    }

	timeout = jiffies;
#endif

	return r;
}

static void __exit dm_verity_exit(void)
{
	dm_unregister_target(&verity_target);
}

module_init(dm_verity_init);
module_exit(dm_verity_exit);

MODULE_AUTHOR("Mikulas Patocka <mpatocka@redhat.com>");
MODULE_AUTHOR("Mandeep Baines <msb@chromium.org>");
MODULE_AUTHOR("Will Drewry <wad@chromium.org>");
MODULE_DESCRIPTION(DM_NAME " target for transparent disk integrity checking");
MODULE_LICENSE("GPL");
