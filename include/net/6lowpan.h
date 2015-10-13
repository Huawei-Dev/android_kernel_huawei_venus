/*
 * Copyright 2011, Siemens AG
 * written by Alexander Smirnov <alex.bluesman.smirnov@gmail.com>
 */

/*
 * Based on patches from Jon Smirl <jonsmirl@gmail.com>
 * Copyright (c) 2011 Jon Smirl <jonsmirl@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* Jon's code is based on 6lowpan implementation for Contiki which is:
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __6LOWPAN_H__
#define __6LOWPAN_H__

#include <net/ipv6.h>
#include <net/net_namespace.h>

#define UIP_802154_SHORTADDR_LEN	2  /* compressed ipv6 address length */
#define UIP_IPH_LEN			40 /* ipv6 fixed header size */
#define UIP_PROTO_UDP			17 /* ipv6 next header value for UDP */
#define UIP_FRAGH_LEN			8  /* ipv6 fragment header size */

#define EUI64_ADDR_LEN		8

#define LOWPAN_NHC_MAX_ID_LEN	1
/* Maximum next header compression length which we currently support inclusive
 * possible inline data.
 */
#define LOWPAN_NHC_MAX_HDR_LEN	(sizeof(struct udphdr))
/* Max IPHC Header len without IPv6 hdr specific inline data.
 * Useful for getting the "extra" bytes we need at worst case compression.
 *
 * LOWPAN_IPHC + CID + LOWPAN_NHC_MAX_ID_LEN
 */
#define LOWPAN_IPHC_MAX_HEADER_LEN	(2 + 1 + LOWPAN_NHC_MAX_ID_LEN)
/* Maximum worst case IPHC header buffer size */
#define LOWPAN_IPHC_MAX_HC_BUF_LEN	(sizeof(struct ipv6hdr) +	\
					 LOWPAN_IPHC_MAX_HEADER_LEN +	\
					 LOWPAN_NHC_MAX_HDR_LEN)

/*
 * ipv6 address based on mac
 * second bit-flip (Universe/Local) is done according RFC2464
 */
#define is_addr_mac_addr_based(a, m) \
	((((a)->s6_addr[8])  == (((m)[0]) ^ 0x02)) &&	\
	 (((a)->s6_addr[9])  == (m)[1]) &&		\
	 (((a)->s6_addr[10]) == (m)[2]) &&		\
	 (((a)->s6_addr[11]) == (m)[3]) &&		\
	 (((a)->s6_addr[12]) == (m)[4]) &&		\
	 (((a)->s6_addr[13]) == (m)[5]) &&		\
	 (((a)->s6_addr[14]) == (m)[6]) &&		\
	 (((a)->s6_addr[15]) == (m)[7]))

/*
 * check whether we can compress the IID to 16 bits,
 * it's possible for unicast adresses with first 49 bits are zero only.
 */
#define lowpan_is_iid_16_bit_compressable(a)	\
	((((a)->s6_addr16[4]) == 0) &&		\
	 (((a)->s6_addr[10]) == 0) &&		\
	 (((a)->s6_addr[11]) == 0xff) &&	\
	 (((a)->s6_addr[12]) == 0xfe) &&	\
	 (((a)->s6_addr[13]) == 0))

/* check whether the 112-bit gid of the multicast address is mappable to: */

/* 48 bits, FFXX::00XX:XXXX:XXXX */
#define lowpan_is_mcast_addr_compressable48(a)	\
	((((a)->s6_addr16[1]) == 0) &&		\
	 (((a)->s6_addr16[2]) == 0) &&		\
	 (((a)->s6_addr16[3]) == 0) &&		\
	 (((a)->s6_addr16[4]) == 0) &&		\
	 (((a)->s6_addr[10]) == 0))

/* 32 bits, FFXX::00XX:XXXX */
#define lowpan_is_mcast_addr_compressable32(a)	\
	((((a)->s6_addr16[1]) == 0) &&		\
	 (((a)->s6_addr16[2]) == 0) &&		\
	 (((a)->s6_addr16[3]) == 0) &&		\
	 (((a)->s6_addr16[4]) == 0) &&		\
	 (((a)->s6_addr16[5]) == 0) &&		\
	 (((a)->s6_addr[12]) == 0))

/* 8 bits, FF02::00XX */
#define lowpan_is_mcast_addr_compressable8(a)	\
	((((a)->s6_addr[1])  == 2) &&		\
	 (((a)->s6_addr16[1]) == 0) &&		\
	 (((a)->s6_addr16[2]) == 0) &&		\
	 (((a)->s6_addr16[3]) == 0) &&		\
	 (((a)->s6_addr16[4]) == 0) &&		\
	 (((a)->s6_addr16[5]) == 0) &&		\
	 (((a)->s6_addr16[6]) == 0) &&		\
	 (((a)->s6_addr[14]) == 0))

#define lowpan_is_addr_broadcast(a)	\
	((((a)[0]) == 0xFF) &&	\
	 (((a)[1]) == 0xFF) &&	\
	 (((a)[2]) == 0xFF) &&	\
	 (((a)[3]) == 0xFF) &&	\
	 (((a)[4]) == 0xFF) &&	\
	 (((a)[5]) == 0xFF) &&	\
	 (((a)[6]) == 0xFF) &&	\
	 (((a)[7]) == 0xFF))

#define LOWPAN_DISPATCH_IPV6		0x41 /* 01000001 = 65 */
#define LOWPAN_DISPATCH_IPHC		0x60 /* 011xxxxx = ... */
#define LOWPAN_DISPATCH_IPHC_MASK	0xe0

static inline bool lowpan_is_ipv6(u8 dispatch)
{
	return dispatch == LOWPAN_DISPATCH_IPV6;
}

static inline bool lowpan_is_iphc(u8 dispatch)
{
	return (dispatch & LOWPAN_DISPATCH_IPHC_MASK) == LOWPAN_DISPATCH_IPHC;
}

#define LOWPAN_FRAG_TIMEOUT	(HZ * 60)	/* time-out 60 sec */

#define LOWPAN_FRAG1_HEAD_SIZE	0x4
#define LOWPAN_FRAGN_HEAD_SIZE	0x5

/*
 * Values of fields within the IPHC encoding first byte
 * (C stands for compressed and I for inline)
 */
#define LOWPAN_IPHC_TF		0x18

#define LOWPAN_IPHC_FL_C	0x10
#define LOWPAN_IPHC_TC_C	0x08
#define LOWPAN_IPHC_NH_C	0x04
#define LOWPAN_IPHC_TTL_1	0x01
#define LOWPAN_IPHC_TTL_64	0x02
#define LOWPAN_IPHC_TTL_255	0x03
#define LOWPAN_IPHC_TTL_I	0x00


/* Values of fields within the IPHC encoding second byte */
#define LOWPAN_IPHC_CID		0x80

#define LOWPAN_IPHC_ADDR_00	0x00
#define LOWPAN_IPHC_ADDR_01	0x01
#define LOWPAN_IPHC_ADDR_02	0x02
#define LOWPAN_IPHC_ADDR_03	0x03

#define LOWPAN_IPHC_SAC		0x40
#define LOWPAN_IPHC_SAM		0x30

#define LOWPAN_IPHC_SAM_BIT	4

#define LOWPAN_IPHC_M		0x08
#define LOWPAN_IPHC_DAC		0x04
#define LOWPAN_IPHC_DAM_00	0x00
#define LOWPAN_IPHC_DAM_01	0x01
#define LOWPAN_IPHC_DAM_10	0x02
#define LOWPAN_IPHC_DAM_11	0x03

#define LOWPAN_IPHC_DAM_BIT	0
/*
 * LOWPAN_UDP encoding (works together with IPHC)
 */
#define LOWPAN_NHC_UDP_MASK		0xF8
#define LOWPAN_NHC_UDP_ID		0xF0
#define LOWPAN_NHC_UDP_CHECKSUMC	0x04
#define LOWPAN_NHC_UDP_CHECKSUMI	0x00

#define LOWPAN_NHC_UDP_4BIT_PORT	0xF0B0
#define LOWPAN_NHC_UDP_4BIT_MASK	0xFFF0
#define LOWPAN_NHC_UDP_8BIT_PORT	0xF000
#define LOWPAN_NHC_UDP_8BIT_MASK	0xFF00

/* values for port compression, _with checksum_ ie bit 5 set to 0 */
#define LOWPAN_NHC_UDP_CS_P_00	0xF0 /* all inline */
#define LOWPAN_NHC_UDP_CS_P_01	0xF1 /* source 16bit inline,
					dest = 0xF0 + 8 bit inline */
#define LOWPAN_NHC_UDP_CS_P_10	0xF2 /* source = 0xF0 + 8bit inline,
					dest = 16 bit inline */
#define LOWPAN_NHC_UDP_CS_P_11	0xF3 /* source & dest = 0xF0B + 4bit inline */
#define LOWPAN_NHC_UDP_CS_C	0x04 /* checksum elided */

#define LOWPAN_PRIV_SIZE(llpriv_size)	\
	(sizeof(struct lowpan_priv) + llpriv_size)

enum lowpan_lltypes {
	LOWPAN_LLTYPE_BTLE,
	LOWPAN_LLTYPE_IEEE802154,
};

struct lowpan_priv {
	enum lowpan_lltypes lltype;

	/* must be last */
	u8 priv[0] __aligned(sizeof(void *));
};

static inline
struct lowpan_priv *lowpan_priv(const struct net_device *dev)
{
	return netdev_priv(dev);
}

struct lowpan_802154_cb {
	u16 d_tag;
	unsigned int d_size;
	u8 d_offset;
};

static inline
struct lowpan_802154_cb *lowpan_802154_cb(const struct sk_buff *skb)
{
	BUILD_BUG_ON(sizeof(struct lowpan_802154_cb) > sizeof(skb->cb));
	return (struct lowpan_802154_cb *)skb->cb;
}

#ifdef DEBUG
/* print data in line */
static inline void raw_dump_inline(const char *caller, char *msg,
				   const unsigned char *buf, int len)
{
	if (msg)
		pr_debug("%s():%s: ", caller, msg);

	print_hex_dump_debug("", DUMP_PREFIX_NONE, 16, 1, buf, len, false);
}

/* print data in a table format:
 *
 * addr: xx xx xx xx xx xx
 * addr: xx xx xx xx xx xx
 * ...
 */
static inline void raw_dump_table(const char *caller, char *msg,
				  const unsigned char *buf, int len)
{
	if (msg)
		pr_debug("%s():%s:\n", caller, msg);

	print_hex_dump_debug("\t", DUMP_PREFIX_OFFSET, 16, 1, buf, len, false);
}
#else
static inline void raw_dump_table(const char *caller, char *msg,
				  const unsigned char *buf, int len) { }
static inline void raw_dump_inline(const char *caller, char *msg,
				   const unsigned char *buf, int len) { }
#endif

/**
 * lowpan_fetch_skb - getting inline data from 6LoWPAN header
 *
 * This function will pull data from sk buffer and put it into data to
 * remove the 6LoWPAN inline data. This function returns true if the
 * sk buffer is too small to pull the amount of data which is specified
 * by len.
 *
 * @skb: the buffer where the inline data should be pulled from.
 * @data: destination buffer for the inline data.
 * @len: amount of data which should be pulled in bytes.
 */
static inline bool lowpan_fetch_skb(struct sk_buff *skb, void *data,
				    unsigned int len)
{
	if (unlikely(!pskb_may_pull(skb, len)))
		return true;

	skb_copy_from_linear_data(skb, data, len);
	skb_pull(skb, len);

	return false;
}

static inline void lowpan_push_hc_data(u8 **hc_ptr, const void *data,
				       const size_t len)
{
	memcpy(*hc_ptr, data, len);
	*hc_ptr += len;
}

void lowpan_netdev_setup(struct net_device *dev, enum lowpan_lltypes lltype);

/**
 * lowpan_header_decompress - replace 6LoWPAN header with IPv6 header
 *
 * This function replaces the IPHC 6LoWPAN header which should be pointed at
 * skb->data and skb_network_header, with the IPv6 header.
 * It would be nice that the caller have the necessary headroom of IPv6 header
 * and greatest Transport layer header, this would reduce the overhead for
 * reallocate headroom.
 *
 * @skb: the buffer which should be manipulate.
 * @dev: the lowpan net device pointer.
 * @daddr: destination lladdr of mac header which is used for compression
 *	methods.
 * @saddr: source lladdr of mac header which is used for compression
 *	methods.
 */
int lowpan_header_decompress(struct sk_buff *skb, const struct net_device *dev,
			     const void *daddr, const void *saddr);

/**
 * lowpan_header_compress - replace IPv6 header with 6LoWPAN header
 *
 * This function replaces the IPv6 header which should be pointed at
 * skb->data and skb_network_header, with the IPHC 6LoWPAN header.
 * The caller need to be sure that the sk buffer is not shared and at have
 * at least a headroom which is smaller or equal LOWPAN_IPHC_MAX_HEADER_LEN,
 * which is the IPHC "more bytes than IPv6 header" at worst case.
 *
 * @skb: the buffer which should be manipulate.
 * @dev: the lowpan net device pointer.
 * @daddr: destination lladdr of mac header which is used for compression
 *	methods.
 * @saddr: source lladdr of mac header which is used for compression
 *	methods.
 */
int lowpan_header_compress(struct sk_buff *skb, const struct net_device *dev,
			   const void *daddr, const void *saddr);

#endif /* __6LOWPAN_H__ */
