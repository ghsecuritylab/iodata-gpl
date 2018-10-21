#ifndef _XT_MAC_H
#define _XT_MAC_H

#include "gconfig.h"

/*
 * 2010-0827 mook:
 *   Sync with header file linux-2.6.30/include/linux/netfilter/xt_mac.h.
 *   They should be the same.
 */

/* 2011-0330 mook,
 * LINUX_VERSION_CODE defined in toolchain may be different with
 * defined in kernel headers. (Ubicom IP8K) */
#if LINUX_VER_CODE > KERNEL_VERSION_NUM(2,6,30)
struct xt_mac_info {
	unsigned char srcaddr[ETH_ALEN];
	int invert;
};

#else /* LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30) */

#define MAC_SRC         0x01    /* Match source MAC address */
#define MAC_DST         0x02    /* Match destination MAC address */
#define MAC_SRC_INV     0x10    /* Negate the condition */
#define MAC_DST_INV     0x20    /* Negate the condition */

struct xt_mac {
	unsigned char macaddr[ETH_ALEN];
}; 

struct xt_mac_info {
	struct xt_mac srcaddr;
	struct xt_mac dstaddr;
	u_int8_t flags;
};
#endif

#endif /*_XT_MAC_H*/
