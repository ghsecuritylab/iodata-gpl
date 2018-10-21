/*
 * Export MIPS-specific functions needed for loadable modules.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 97, 98, 99, 2000, 01, 03, 04, 05 by Ralf Baechle
 * Copyright (C) 1999, 2000, 01 Silicon Graphics, Inc.
 */
#include <linux/interrupt.h>
#include <linux/module.h>
#include <asm/checksum.h>
#include <asm/pgtable.h>
#include <asm/uaccess.h>
#include <linux/skbuff.h>

#include <linux/if.h>
#include "gconfig.h"

extern void *__bzero(void *__s, size_t __count);
extern long __strncpy_from_user_nocheck_asm(char *__to,
                                            const char *__from, long __len);
extern long __strncpy_from_user_asm(char *__to, const char *__from,
                                    long __len);
extern long __strlen_user_nocheck_asm(const char *s);
extern long __strlen_user_asm(const char *s);
extern long __strnlen_user_nocheck_asm(const char *s);
extern long __strnlen_user_asm(const char *s);

/*
 * String functions
 */
EXPORT_SYMBOL(memset);
EXPORT_SYMBOL(memcpy);
EXPORT_SYMBOL(memmove);

EXPORT_SYMBOL(kernel_thread);

/*
 * Userspace access stuff.
 */
EXPORT_SYMBOL(__copy_user);
EXPORT_SYMBOL(__copy_user_inatomic);
EXPORT_SYMBOL(__bzero);
EXPORT_SYMBOL(__strncpy_from_user_nocheck_asm);
EXPORT_SYMBOL(__strncpy_from_user_asm);
EXPORT_SYMBOL(__strlen_user_nocheck_asm);
EXPORT_SYMBOL(__strlen_user_asm);
EXPORT_SYMBOL(__strnlen_user_nocheck_asm);
EXPORT_SYMBOL(__strnlen_user_asm);

EXPORT_SYMBOL(csum_partial);
EXPORT_SYMBOL(csum_partial_copy_nocheck);
EXPORT_SYMBOL(__csum_partial_copy_user);

EXPORT_SYMBOL(invalid_pte_table);

#ifdef CONFIG_IP_CONNTRACK_NAT_SESSION_RESERVATION
extern int (*dropWhenNatTableFull_Ptr)(struct sk_buff *skb, int natSession, int natSessionMax );
extern int (*newConntrackDropSelect_Ptr)(u_int8_t protonum, u_int16_t tcp_port, u_int16_t udp_port);
EXPORT_SYMBOL(dropWhenNatTableFull_Ptr);
EXPORT_SYMBOL(newConntrackDropSelect_Ptr);
#endif

#ifdef CONFIG_SUPPORT_CHT_IPV6_LOGO_TEST
extern int (*cht_ip6pkthandler_rx_handler)(struct sk_buff *skb);
EXPORT_SYMBOL(cht_ip6pkthandler_rx_handler);

extern int (*cht_ip6pkthandler_tx_handler)(struct sk_buff *skb);
EXPORT_SYMBOL(cht_ip6pkthandler_tx_handler);
#endif

#ifdef CONFIG_NPH_MODULE
extern int (*nph_rx_handler)(struct sk_buff *skb);
EXPORT_SYMBOL(nph_rx_handler);

extern int (*nph_tx_handler)(struct sk_buff *skb);
EXPORT_SYMBOL(nph_tx_handler);
#endif

#ifdef CONFIG_PACKET_HANDLER

extern int (*netpkthandler_handle_driver)(struct sk_buff *skb);
EXPORT_SYMBOL(netpkthandler_handle_driver);

extern int (*netpkthandler_handle_tx_driver)(struct sk_buff *skb);
EXPORT_SYMBOL(netpkthandler_handle_tx_driver);

extern char wanMacAddress[20];
EXPORT_SYMBOL(wanMacAddress);

#ifdef CONFIG_PACKET_HANDLER_PASSTHROUGH
extern int (*addInterface_Ptr)(struct ifreq *ifr);
EXPORT_SYMBOL(addInterface_Ptr);
#endif /* CONFIG_PACKET_HANDLER_PASSTHROUGH */

#endif /* CONFIG_PACKET_HANDLER */

