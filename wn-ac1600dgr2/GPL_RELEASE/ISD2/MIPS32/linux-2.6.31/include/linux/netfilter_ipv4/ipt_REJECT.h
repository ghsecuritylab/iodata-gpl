#ifndef _IPT_REJECT_H
#define _IPT_REJECT_H

enum ipt_reject_with {
	IPT_ICMP_NET_UNREACHABLE,
	IPT_ICMP_HOST_UNREACHABLE,
	IPT_ICMP_PROT_UNREACHABLE,
	IPT_ICMP_PORT_UNREACHABLE,
	IPT_ICMP_ECHOREPLY,
	IPT_ICMP_NET_PROHIBITED,
	IPT_ICMP_HOST_PROHIBITED,
	IPT_TCP_RESET,
	IPT_ICMP_ADMIN_PROHIBITED,
	IPT_TCP_HTTP_REDIRECT
};

struct ipt_reject_info {
	enum ipt_reject_with with;      /* reject type */
	char toUrl[50]; /* redirect to URL */

};

#endif /*_IPT_REJECT_H*/
