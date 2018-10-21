/* Kernel module to match the port-ranges, trigger related port-ranges,
 * and alters the destination to a local IP address.
 *
 * Copyright (C) 2003, CyberTAN Corporation
 * All Rights Reserved.
 *
 * Description:
 *   This is kernel module for port-triggering.
 *
 *   The module follows the Netfilter framework, called extended packet
 *   matching modules.
 */

#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/inetdevice.h>
#include <net/protocol.h>
#include <net/checksum.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_nat_rule.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_TRIGGER.h>

/* Return pointer to first true entry, if any, or NULL. A macro
 *    required to allow inlining of cmpfn. */
#define LIST_FIND(head, cmpfn, type, args...)		\
({							\
	const struct list_head *__i, *__j = NULL;	\
							\
	list_for_each(__i, (head))			\
		if (cmpfn((const type)__i , ## args)) {	\
			__j = __i;			\
			break;				\
		}					\
	(type)__j;					\
})

#define DUMP_TUPLE(tp)						\
pr_debug("tuple %p: %u %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u\n",	\
	(tp), (tp)->dst.protonum,				\
	NIPQUAD((tp)->src.u3.ip), ntohl((tp)->src.u.all),		\
	NIPQUAD((tp)->dst.u3.ip), ntohl((tp)->dst.u.all))

struct ipt_trigger {
	struct list_head list;		/* Trigger list */
	struct timer_list timeout;	/* Timer for list destroying */
	u_int32_t srcip;		/* Outgoing source address */
	u_int32_t dstip;		/* Outgoing destination address */
	u_int16_t mproto;		/* Trigger protocol */
	u_int16_t rproto;		/* Related protocol */
	struct ipt_trigger_ports ports;	/* Trigger and related ports */
	u_int8_t reply;			/* Confirm a reply connection */
};

LIST_HEAD(trigger_list);

static void trigger_refresh(struct ipt_trigger *trig, unsigned long extra_jiffies)
{
	pr_debug("%s: \n", __FUNCTION__);
	NF_CT_ASSERT(trig);
	spin_lock_bh(&nf_conntrack_lock);

	/* Need del_timer for race avoidance (may already be dying). */
	if (del_timer(&trig->timeout)) {
		trig->timeout.expires = jiffies + extra_jiffies;
		add_timer(&trig->timeout);
	}

	spin_unlock_bh(&nf_conntrack_lock);
}

static void __del_trigger(struct ipt_trigger *trig)
{
	pr_debug("%s: \n", __FUNCTION__);
	NF_CT_ASSERT(trig);
	spin_lock_bh(&nf_conntrack_lock);
	/* delete from 'trigger_list' */
	list_del(&trig->list);
	spin_unlock_bh(&nf_conntrack_lock);
	kfree(trig);
}

static void trigger_timeout(unsigned long ul_trig)
{
	struct ipt_trigger *trig= (void *) ul_trig;

	pr_debug("trigger list %p timed out\n", trig);
	spin_lock_bh(&nf_conntrack_lock);
	__del_trigger(trig);
	spin_unlock_bh(&nf_conntrack_lock);
}

static unsigned int
add_new_trigger(struct ipt_trigger *trig)
{
	struct ipt_trigger *new;

	pr_debug("!!!!!!!!!!!! %s !!!!!!!!!!!\n", __FUNCTION__);
	spin_lock_bh(&nf_conntrack_lock);
	new = (struct ipt_trigger *)
	kmalloc(sizeof(struct ipt_trigger), GFP_ATOMIC);

	if (!new) {
		spin_unlock_bh(&nf_conntrack_lock);
		pr_debug("%s: OOM allocating trigger list\n", __FUNCTION__);
		return -ENOMEM;
	}

	memset(new, 0, sizeof(*trig));
	INIT_LIST_HEAD(&new->list);
	memcpy(new, trig, sizeof(*trig));

	/* add to global table of trigger */
	list_add(&new->list, &trigger_list);
	/* add and start timer if required */
	init_timer(&new->timeout);
	new->timeout.data = (unsigned long)new;
	new->timeout.function = trigger_timeout;
	new->timeout.expires = jiffies + (TRIGGER_TIMEOUT * HZ);
	add_timer(&new->timeout);

	spin_unlock_bh(&nf_conntrack_lock);

	return 0;
}

static inline int trigger_out_matched(const struct ipt_trigger *i,
		const u_int16_t proto, const u_int16_t dport, const struct ipt_trigger_info *info)
{
	int ret;
	/* DEBUGP("%s: i=%p, proto= %d, dport=%d.\n", __FUNCTION__, i, proto, dport);
 * 	   DEBUGP("%s: Got one, mproto= %d, mport[0..1]=%d, %d.\n", __FUNCTION__, 
 * 	   	   i->mproto, i->ports.mport[0], i->ports.mport[1]); */

	ret = ((i->mproto == proto) &&
	       (i->ports.mport[0] <= dport) &&
	       (i->ports.mport[1] >= dport));

	/* We should confirm targinfo == i, sometimes one mport map into many rports  */
	ret = ret &&
	      (info->ports.mport[0] == i->ports.mport[0]) &&
	      (info->ports.mport[1] == i->ports.mport[1]) &&
	      (info->ports.rport[0] == i->ports.rport[0]) &&
	      (info->ports.rport[1] == i->ports.rport[1]);

	return ret;
}

static unsigned int
trigger_out(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct ipt_trigger_info *info = par->targinfo;
	struct ipt_trigger trig, *found;
	const struct iphdr *iph = ip_hdr(skb);
	struct tcphdr *tcph = (void *)iph + iph->ihl*4;	/* Might be TCP, UDP */

	pr_debug("############# %s ############\n", __FUNCTION__);
	/* Check if the trigger range has already existed in 'trigger_list'. */
	found = LIST_FIND(&trigger_list, trigger_out_matched,
		struct ipt_trigger *, iph->protocol, ntohs(tcph->dest), info);

	if (found) {
		/* Yeah, it exists. We need to update(delay) the destroying timer. */
		trigger_refresh(found, TRIGGER_TIMEOUT * HZ);
		/* In order to allow multiple hosts use the same port range, we update
 * 		   the 'saddr' after previous trigger has a reply connection. */
		if (found->reply)
			found->srcip = iph->saddr;
		}
	else {
		if (((info->proto == 0) || (info->proto == iph->protocol)) &&
		    (info->ports.mport[0] <= ntohs(tcph->dest)) &&
		    (info->ports.mport[1] >= ntohs(tcph->dest)))
		{
		/* Create new trigger */
		memset(&trig, 0, sizeof(trig));
		trig.srcip = iph->saddr;
		trig.mproto = iph->protocol;
		trig.rproto = 0;
		memcpy(&trig.ports, &info->ports, sizeof(struct ipt_trigger_ports));
		add_new_trigger(&trig);	/* Add the new 'trig' to list 'trigger_list'. */
	}
	}

	return IPT_CONTINUE;	/* We don't block any packet. */
}

static inline int trigger_in_matched(const struct ipt_trigger *i,
	const u_int16_t proto, const u_int16_t dport)
{
	u_int16_t rproto = i->rproto;

	/*
 * 	pr_debug("%s: i=%p, proto= %d, dport=%d.\n", __FUNCTION__, i, proto, dport);
 * 		pr_debug("%s: Got one, rproto= %d, rport[0..1]=%d, %d.\n", __FUNCTION__,
 * 				i->rproto, i->ports.rport[0], i->ports.rport[1]); */

	if (!rproto)
		rproto = proto;

	return ((rproto == proto) && (i->ports.rport[0] <= dport)
		&& (i->ports.rport[1] >= dport));
}

static unsigned int
trigger_in(struct sk_buff *skb, const struct xt_target_param *par)
{
	struct ipt_trigger *found;
	const struct iphdr *iph = ip_hdr(skb);
	struct tcphdr *tcph = (void *)iph + iph->ihl*4;	/* Might be TCP, UDP */
	/* Check if the trigger-ed range has already existed in 'trigger_list'. */
	found = LIST_FIND(&trigger_list, trigger_in_matched,
		struct ipt_trigger *, iph->protocol, ntohs(tcph->dest));
	if (found) {
		pr_debug("############# %s ############\n", __FUNCTION__);
		/* Yeah, it exists. We need to update(delay) the destroying timer. */
		trigger_refresh(found, TRIGGER_TIMEOUT * HZ);
		return NF_ACCEPT;	/* Accept it, or the imcoming packet could be dropped in the FORWARD chain */
	}

	return IPT_CONTINUE;	/* Our job is the interception. */
}

static unsigned int
trigger_dnat(struct sk_buff *skb, const struct xt_target_param *par)
{
	struct ipt_trigger *found;
	const struct iphdr *iph = ip_hdr(skb);
	struct tcphdr *tcph = (void *)iph + iph->ihl*4;	/* Might be TCP, UDP */
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	struct nf_nat_multi_range_compat newrange;

	NF_CT_ASSERT(par->hooknum == NF_INET_PRE_ROUTING);
	/* Check if the trigger-ed range has already existed in 'trigger_list'. */
	found = LIST_FIND(&trigger_list, trigger_in_matched,
		struct ipt_trigger *, iph->protocol, ntohs(tcph->dest));

	if (!found || !found->srcip)
		return IPT_CONTINUE;	/* We don't block any packet. */

	pr_debug("############# %s ############\n", __FUNCTION__);
	found->reply = 1;	/* Confirm there has been a reply connection. */
	ct = nf_ct_get(skb, &ctinfo);
	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW));

	pr_debug("%s: got ", __FUNCTION__);
	DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);

	/* Alter the destination of imcoming packet. */
	newrange = ((struct nf_nat_multi_range_compat)
		{ 1, { { IP_NAT_RANGE_MAP_IPS,
			found->srcip, found->srcip,
			{ 0 }, { 0 }
		} } });

	/* Hand modified range to generic setup. */
	return nf_nat_setup_info(ct, &newrange.range[0], HOOK2MANIP(par->hooknum));
}

static unsigned int
trigger_target(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct ipt_trigger_info *info = par->targinfo;
	const struct iphdr *iph = ip_hdr(skb);

	/* pr_debug("%s: type = %s\n", __FUNCTION__,
 * 		(info->type == IPT_TRIGGER_DNAT) ? "dnat" :
 * 				(info->type == IPT_TRIGGER_IN) ? "in" : "out"); */

	/* The Port-trigger only supports TCP and UDP. */
	if ((iph->protocol != IPPROTO_TCP) && (iph->protocol != IPPROTO_UDP))
		return IPT_CONTINUE;

	if (info->type == IPT_TRIGGER_OUT)
		return trigger_out(skb, par);
	else if (info->type == IPT_TRIGGER_IN)
		return trigger_in(skb, par);
	else if (info->type == IPT_TRIGGER_DNAT)
		return trigger_dnat(skb, par);

	return IPT_CONTINUE;
}
static bool
trigger_check(const struct xt_tgchk_param *par)
{
	const struct ipt_trigger_info *info = par->targinfo;
	struct list_head *cur_item, *tmp_item;

	if ((strcmp(par->table, "mangle") == 0)) {
		pr_debug("trigger_check: bad table `%s'.\n", par->table);
		return false;
	}
	if (info->proto && (info->proto != IPPROTO_TCP && info->proto != IPPROTO_UDP)) {
		pr_debug("trigger_check: bad proto %d.\n", info->proto);
		return false;
	}
	if (info->type == IPT_TRIGGER_OUT && (!info->ports.mport[0] || !info->ports.rport[0])) {
		pr_debug("trigger_check: Try 'iptbles -j TRIGGER -h' for help.\n");
		return false;
	}

	/* Empty the 'trigger_list' */
	list_for_each_safe(cur_item, tmp_item, &trigger_list) {
		struct ipt_trigger *trig = (void *)cur_item;

		pr_debug("%s: list_for_each_safe(): %p.\n", __FUNCTION__, trig);
		del_timer(&trig->timeout);
		__del_trigger(trig);
	}

	return true;
}

static struct xt_target trigger_reg = {
	.name 		= "TRIGGER",
	.family		= AF_INET,
	.target 	= trigger_target,
	.targetsize	= sizeof(struct ipt_trigger_info),
	.hooks		= (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_FORWARD),
	.checkentry 	= trigger_check,
	.me 		= THIS_MODULE,
};

static int __init init(void)
{
	return xt_register_target(&trigger_reg);
}

static void __exit fini(void)
{
	xt_unregister_target(&trigger_reg);
}

module_init(init);
module_exit(fini);

