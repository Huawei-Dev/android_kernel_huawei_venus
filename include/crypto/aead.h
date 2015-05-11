/*
 * AEAD: Authenticated Encryption with Associated Data
 * 
 * Copyright (c) 2007 Herbert Xu <herbert@gondor.apana.org.au>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) 
 * any later version.
 *
 */

#ifndef _CRYPTO_AEAD_H
#define _CRYPTO_AEAD_H

#include <linux/crypto.h>
#include <linux/kernel.h>
#include <linux/slab.h>

/**
 * DOC: Authenticated Encryption With Associated Data (AEAD) Cipher API
 *
 * The AEAD cipher API is used with the ciphers of type CRYPTO_ALG_TYPE_AEAD
 * (listed as type "aead" in /proc/crypto)
 *
 * The most prominent examples for this type of encryption is GCM and CCM.
 * However, the kernel supports other types of AEAD ciphers which are defined
 * with the following cipher string:
 *
 *	authenc(keyed message digest, block cipher)
 *
 * For example: authenc(hmac(sha256), cbc(aes))
 *
 * The example code provided for the asynchronous block cipher operation
 * applies here as well. Naturally all *ablkcipher* symbols must be exchanged
 * the *aead* pendants discussed in the following. In addtion, for the AEAD
 * operation, the aead_request_set_assoc function must be used to set the
 * pointer to the associated data memory location before performing the
 * encryption or decryption operation. In case of an encryption, the associated
 * data memory is filled during the encryption operation. For decryption, the
 * associated data memory must contain data that is used to verify the integrity
 * of the decrypted data. Another deviation from the asynchronous block cipher
 * operation is that the caller should explicitly check for -EBADMSG of the
 * crypto_aead_decrypt. That error indicates an authentication error, i.e.
 * a breach in the integrity of the message. In essence, that -EBADMSG error
 * code is the key bonus an AEAD cipher has over "standard" block chaining
 * modes.
 */

/**
 *	struct aead_request - AEAD request
 *	@base: Common attributes for async crypto requests
 *	@assoclen: Length in bytes of associated data for authentication
 *	@cryptlen: Length of data to be encrypted or decrypted
 *	@iv: Initialisation vector
 *	@assoc: Associated data
 *	@src: Source data
 *	@dst: Destination data
 *	@__ctx: Start of private context data
 */
struct aead_request {
	struct crypto_async_request base;

	unsigned int assoclen;
	unsigned int cryptlen;

	u8 *iv;

	struct scatterlist *assoc;
	struct scatterlist *src;
	struct scatterlist *dst;

	void *__ctx[] CRYPTO_MINALIGN_ATTR;
};

/**
 *	struct aead_givcrypt_request - AEAD request with IV generation
 *	@seq: Sequence number for IV generation
 *	@giv: Space for generated IV
 *	@areq: The AEAD request itself
 */
struct aead_givcrypt_request {
	u64 seq;
	u8 *giv;

	struct aead_request areq;
};

struct crypto_aead {
	int (*encrypt)(struct aead_request *req);
	int (*decrypt)(struct aead_request *req);
	int (*givencrypt)(struct aead_givcrypt_request *req);
	int (*givdecrypt)(struct aead_givcrypt_request *req);

	struct crypto_aead *child;

	unsigned int ivsize;
	unsigned int authsize;
	unsigned int reqsize;

	struct crypto_tfm base;
};

static inline struct crypto_aead *__crypto_aead_cast(struct crypto_tfm *tfm)
{
	return container_of(tfm, struct crypto_aead, base);
}

/**
 * crypto_alloc_aead() - allocate AEAD cipher handle
 * @alg_name: is the cra_name / name or cra_driver_name / driver name of the
 *	     AEAD cipher
 * @type: specifies the type of the cipher
 * @mask: specifies the mask for the cipher
 *
 * Allocate a cipher handle for an AEAD. The returned struct
 * crypto_aead is the cipher handle that is required for any subsequent
 * API invocation for that AEAD.
 *
 * Return: allocated cipher handle in case of success; IS_ERR() is true in case
 *	   of an error, PTR_ERR() returns the error code.
 */
struct crypto_aead *crypto_alloc_aead(const char *alg_name, u32 type, u32 mask);

static inline struct crypto_tfm *crypto_aead_tfm(struct crypto_aead *tfm)
{
	return &tfm->base;
}

/**
 * crypto_free_aead() - zeroize and free aead handle
 * @tfm: cipher handle to be freed
 */
static inline void crypto_free_aead(struct crypto_aead *tfm)
{
	crypto_destroy_tfm(tfm, crypto_aead_tfm(tfm));
}

static inline struct crypto_aead *crypto_aead_crt(struct crypto_aead *tfm)
{
	return tfm;
}

/**
 * crypto_aead_ivsize() - obtain IV size
 * @tfm: cipher handle
 *
 * The size of the IV for the aead referenced by the cipher handle is
 * returned. This IV size may be zero if the cipher does not need an IV.
 *
 * Return: IV size in bytes
 */
static inline unsigned int crypto_aead_ivsize(struct crypto_aead *tfm)
{
	return tfm->ivsize;
}

/**
 * crypto_aead_authsize() - obtain maximum authentication data size
 * @tfm: cipher handle
 *
 * The maximum size of the authentication data for the AEAD cipher referenced
 * by the AEAD cipher handle is returned. The authentication data size may be
 * zero if the cipher implements a hard-coded maximum.
 *
 * The authentication data may also be known as "tag value".
 *
 * Return: authentication data size / tag size in bytes
 */
static inline unsigned int crypto_aead_authsize(struct crypto_aead *tfm)
{
	return tfm->authsize;
}

/**
 * crypto_aead_blocksize() - obtain block size of cipher
 * @tfm: cipher handle
 *
 * The block size for the AEAD referenced with the cipher handle is returned.
 * The caller may use that information to allocate appropriate memory for the
 * data returned by the encryption or decryption operation
 *
 * Return: block size of cipher
 */
static inline unsigned int crypto_aead_blocksize(struct crypto_aead *tfm)
{
	return crypto_tfm_alg_blocksize(crypto_aead_tfm(tfm));
}

static inline unsigned int crypto_aead_alignmask(struct crypto_aead *tfm)
{
	return crypto_tfm_alg_alignmask(crypto_aead_tfm(tfm));
}

static inline u32 crypto_aead_get_flags(struct crypto_aead *tfm)
{
	return crypto_tfm_get_flags(crypto_aead_tfm(tfm));
}

static inline void crypto_aead_set_flags(struct crypto_aead *tfm, u32 flags)
{
	crypto_tfm_set_flags(crypto_aead_tfm(tfm), flags);
}

static inline void crypto_aead_clear_flags(struct crypto_aead *tfm, u32 flags)
{
	crypto_tfm_clear_flags(crypto_aead_tfm(tfm), flags);
}

/**
 * crypto_aead_setkey() - set key for cipher
 * @tfm: cipher handle
 * @key: buffer holding the key
 * @keylen: length of the key in bytes
 *
 * The caller provided key is set for the AEAD referenced by the cipher
 * handle.
 *
 * Note, the key length determines the cipher type. Many block ciphers implement
 * different cipher modes depending on the key size, such as AES-128 vs AES-192
 * vs. AES-256. When providing a 16 byte key for an AES cipher handle, AES-128
 * is performed.
 *
 * Return: 0 if the setting of the key was successful; < 0 if an error occurred
 */
int crypto_aead_setkey(struct crypto_aead *tfm,
		       const u8 *key, unsigned int keylen);

/**
 * crypto_aead_setauthsize() - set authentication data size
 * @tfm: cipher handle
 * @authsize: size of the authentication data / tag in bytes
 *
 * Set the authentication data size / tag size. AEAD requires an authentication
 * tag (or MAC) in addition to the associated data.
 *
 * Return: 0 if the setting of the key was successful; < 0 if an error occurred
 */
int crypto_aead_setauthsize(struct crypto_aead *tfm, unsigned int authsize);

static inline struct crypto_aead *crypto_aead_reqtfm(struct aead_request *req)
{
	return __crypto_aead_cast(req->base.tfm);
}

/**
 * crypto_aead_encrypt() - encrypt plaintext
 * @req: reference to the aead_request handle that holds all information
 *	 needed to perform the cipher operation
 *
 * Encrypt plaintext data using the aead_request handle. That data structure
 * and how it is filled with data is discussed with the aead_request_*
 * functions.
 *
 * IMPORTANT NOTE The encryption operation creates the authentication data /
 *		  tag. That data is concatenated with the created ciphertext.
 *		  The ciphertext memory size is therefore the given number of
 *		  block cipher blocks + the size defined by the
 *		  crypto_aead_setauthsize invocation. The caller must ensure
 *		  that sufficient memory is available for the ciphertext and
 *		  the authentication tag.
 *
 * Return: 0 if the cipher operation was successful; < 0 if an error occurred
 */
static inline int crypto_aead_encrypt(struct aead_request *req)
{
	return crypto_aead_reqtfm(req)->encrypt(req);
}

/**
 * crypto_aead_decrypt() - decrypt ciphertext
 * @req: reference to the ablkcipher_request handle that holds all information
 *	 needed to perform the cipher operation
 *
 * Decrypt ciphertext data using the aead_request handle. That data structure
 * and how it is filled with data is discussed with the aead_request_*
 * functions.
 *
 * IMPORTANT NOTE The caller must concatenate the ciphertext followed by the
 *		  authentication data / tag. That authentication data / tag
 *		  must have the size defined by the crypto_aead_setauthsize
 *		  invocation.
 *
 *
 * Return: 0 if the cipher operation was successful; -EBADMSG: The AEAD
 *	   cipher operation performs the authentication of the data during the
 *	   decryption operation. Therefore, the function returns this error if
 *	   the authentication of the ciphertext was unsuccessful (i.e. the
 *	   integrity of the ciphertext or the associated data was violated);
 *	   < 0 if an error occurred.
 */
static inline int crypto_aead_decrypt(struct aead_request *req)
{
	if (req->cryptlen < crypto_aead_authsize(crypto_aead_reqtfm(req)))
		return -EINVAL;

	return crypto_aead_reqtfm(req)->decrypt(req);
}

/**
 * DOC: Asynchronous AEAD Request Handle
 *
 * The aead_request data structure contains all pointers to data required for
 * the AEAD cipher operation. This includes the cipher handle (which can be
 * used by multiple aead_request instances), pointer to plaintext and
 * ciphertext, asynchronous callback function, etc. It acts as a handle to the
 * aead_request_* API calls in a similar way as AEAD handle to the
 * crypto_aead_* API calls.
 */

/**
 * crypto_aead_reqsize() - obtain size of the request data structure
 * @tfm: cipher handle
 *
 * Return: number of bytes
 */
static inline unsigned int crypto_aead_reqsize(struct crypto_aead *tfm)
{
	return tfm->reqsize;
}

/**
 * aead_request_set_tfm() - update cipher handle reference in request
 * @req: request handle to be modified
 * @tfm: cipher handle that shall be added to the request handle
 *
 * Allow the caller to replace the existing aead handle in the request
 * data structure with a different one.
 */
static inline void aead_request_set_tfm(struct aead_request *req,
					struct crypto_aead *tfm)
{
	req->base.tfm = crypto_aead_tfm(tfm->child);
}

/**
 * aead_request_alloc() - allocate request data structure
 * @tfm: cipher handle to be registered with the request
 * @gfp: memory allocation flag that is handed to kmalloc by the API call.
 *
 * Allocate the request data structure that must be used with the AEAD
 * encrypt and decrypt API calls. During the allocation, the provided aead
 * handle is registered in the request data structure.
 *
 * Return: allocated request handle in case of success; IS_ERR() is true in case
 *	   of an error, PTR_ERR() returns the error code.
 */
static inline struct aead_request *aead_request_alloc(struct crypto_aead *tfm,
						      gfp_t gfp)
{
	struct aead_request *req;

	req = kmalloc(sizeof(*req) + crypto_aead_reqsize(tfm), gfp);

	if (likely(req))
		aead_request_set_tfm(req, tfm);

	return req;
}

/**
 * aead_request_free() - zeroize and free request data structure
 * @req: request data structure cipher handle to be freed
 */
static inline void aead_request_free(struct aead_request *req)
{
	kzfree(req);
}

/**
 * aead_request_set_callback() - set asynchronous callback function
 * @req: request handle
 * @flags: specify zero or an ORing of the flags
 *	   CRYPTO_TFM_REQ_MAY_BACKLOG the request queue may back log and
 *	   increase the wait queue beyond the initial maximum size;
 *	   CRYPTO_TFM_REQ_MAY_SLEEP the request processing may sleep
 * @compl: callback function pointer to be registered with the request handle
 * @data: The data pointer refers to memory that is not used by the kernel
 *	  crypto API, but provided to the callback function for it to use. Here,
 *	  the caller can provide a reference to memory the callback function can
 *	  operate on. As the callback function is invoked asynchronously to the
 *	  related functionality, it may need to access data structures of the
 *	  related functionality which can be referenced using this pointer. The
 *	  callback function can access the memory via the "data" field in the
 *	  crypto_async_request data structure provided to the callback function.
 *
 * Setting the callback function that is triggered once the cipher operation
 * completes
 *
 * The callback function is registered with the aead_request handle and
 * must comply with the following template
 *
 *	void callback_function(struct crypto_async_request *req, int error)
 */
static inline void aead_request_set_callback(struct aead_request *req,
					     u32 flags,
					     crypto_completion_t compl,
					     void *data)
{
	req->base.complete = compl;
	req->base.data = data;
	req->base.flags = flags;
}

/**
 * aead_request_set_crypt - set data buffers
 * @req: request handle
 * @src: source scatter / gather list
 * @dst: destination scatter / gather list
 * @cryptlen: number of bytes to process from @src
 * @iv: IV for the cipher operation which must comply with the IV size defined
 *      by crypto_aead_ivsize()
 *
 * Setting the source data and destination data scatter / gather lists.
 *
 * For encryption, the source is treated as the plaintext and the
 * destination is the ciphertext. For a decryption operation, the use is
 * reversed - the source is the ciphertext and the destination is the plaintext.
 *
 * IMPORTANT NOTE AEAD requires an authentication tag (MAC). For decryption,
 *		  the caller must concatenate the ciphertext followed by the
 *		  authentication tag and provide the entire data stream to the
 *		  decryption operation (i.e. the data length used for the
 *		  initialization of the scatterlist and the data length for the
 *		  decryption operation is identical). For encryption, however,
 *		  the authentication tag is created while encrypting the data.
 *		  The destination buffer must hold sufficient space for the
 *		  ciphertext and the authentication tag while the encryption
 *		  invocation must only point to the plaintext data size. The
 *		  following code snippet illustrates the memory usage
 *		  buffer = kmalloc(ptbuflen + (enc ? authsize : 0));
 *		  sg_init_one(&sg, buffer, ptbuflen + (enc ? authsize : 0));
 *		  aead_request_set_crypt(req, &sg, &sg, ptbuflen, iv);
 */
static inline void aead_request_set_crypt(struct aead_request *req,
					  struct scatterlist *src,
					  struct scatterlist *dst,
					  unsigned int cryptlen, u8 *iv)
{
	req->src = src;
	req->dst = dst;
	req->cryptlen = cryptlen;
	req->iv = iv;
}

/**
 * aead_request_set_assoc() - set the associated data scatter / gather list
 * @req: request handle
 * @assoc: associated data scatter / gather list
 * @assoclen: number of bytes to process from @assoc
 *
 * For encryption, the memory is filled with the associated data. For
 * decryption, the memory must point to the associated data.
 */
static inline void aead_request_set_assoc(struct aead_request *req,
					  struct scatterlist *assoc,
					  unsigned int assoclen)
{
	req->assoc = assoc;
	req->assoclen = assoclen;
}

static inline struct crypto_aead *aead_givcrypt_reqtfm(
	struct aead_givcrypt_request *req)
{
	return crypto_aead_reqtfm(&req->areq);
}

static inline int crypto_aead_givencrypt(struct aead_givcrypt_request *req)
{
	return aead_givcrypt_reqtfm(req)->givencrypt(req);
};

static inline int crypto_aead_givdecrypt(struct aead_givcrypt_request *req)
{
	return aead_givcrypt_reqtfm(req)->givdecrypt(req);
};

static inline void aead_givcrypt_set_tfm(struct aead_givcrypt_request *req,
					 struct crypto_aead *tfm)
{
	req->areq.base.tfm = crypto_aead_tfm(tfm);
}

static inline struct aead_givcrypt_request *aead_givcrypt_alloc(
	struct crypto_aead *tfm, gfp_t gfp)
{
	struct aead_givcrypt_request *req;

	req = kmalloc(sizeof(struct aead_givcrypt_request) +
		      crypto_aead_reqsize(tfm), gfp);

	if (likely(req))
		aead_givcrypt_set_tfm(req, tfm);

	return req;
}

static inline void aead_givcrypt_free(struct aead_givcrypt_request *req)
{
	kfree(req);
}

static inline void aead_givcrypt_set_callback(
	struct aead_givcrypt_request *req, u32 flags,
	crypto_completion_t compl, void *data)
{
	aead_request_set_callback(&req->areq, flags, compl, data);
}

static inline void aead_givcrypt_set_crypt(struct aead_givcrypt_request *req,
					   struct scatterlist *src,
					   struct scatterlist *dst,
					   unsigned int nbytes, void *iv)
{
	aead_request_set_crypt(&req->areq, src, dst, nbytes, iv);
}

static inline void aead_givcrypt_set_assoc(struct aead_givcrypt_request *req,
					   struct scatterlist *assoc,
					   unsigned int assoclen)
{
	aead_request_set_assoc(&req->areq, assoc, assoclen);
}

static inline void aead_givcrypt_set_giv(struct aead_givcrypt_request *req,
					 u8 *giv, u64 seq)
{
	req->giv = giv;
	req->seq = seq;
}

#endif	/* _CRYPTO_AEAD_H */
