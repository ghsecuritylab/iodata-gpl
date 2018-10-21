/* Shared library add-on to iptables to add MAC address support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <xtables.h>
#include <linux/netfilter/xt_mac.h>

#if HAS_IPTABLES_HELP
static void mac_help(void)
{
	printf(
"mac match options:\n"
"[!] --mac-source XX:XX:XX:XX:XX:XX\n"
"				Match source MAC address\n");
}
#endif

static const struct option mac_opts[] = {
	{ "mac-source", 1, NULL, '1' },
	{ .name = NULL }
};

static void
#if LINUX_VER_CODE > KERNEL_VERSION_NUM(2,6,30)
parse_mac(const char *mac, struct xt_mac_info *info)
#else
parse_mac(const char *mac, struct xt_mac *addr)
#endif
{
	unsigned int i = 0;

	if (strlen(mac) != ETH_ALEN*3-1)
		xtables_error(PARAMETER_PROBLEM, "Bad mac address \"%s\"", mac);

	for (i = 0; i < ETH_ALEN; i++) {
		long number;
		char *end;

		number = strtol(mac + i*3, &end, 16);

		if (end == mac + i*3 + 2
		    && number >= 0
		    && number <= 255)
			/* 2010-0827 mook:
			 * Sync with netfilter xt_mac extension. And realtek 
			 * flatform use modified xt_mac_info of linux-2.6.30.
			 * TODO: need to change following definition if you
			 * are not under realtek flatform. */
#if LINUX_VER_CODE > KERNEL_VERSION_NUM(2,6,30)
			info->srcaddr[i] = number;
#else
			addr->macaddr[i] = number;
#endif
		else
			xtables_error(PARAMETER_PROBLEM,
				   "Bad mac address `%s'", mac);
	}
}

static int
mac_parse(int c, char **argv, int invert, unsigned int *flags,
          const void *entry, struct xt_entry_match **match)
{
	struct xt_mac_info *macinfo = (struct xt_mac_info *)(*match)->data;

	switch (c) {
	case '1':
		xtables_check_inverse(optarg, &invert, &optind, 0);
		/* 2010-0827 mook:
		 *   Sync with netfilter xt_mac extension. */
#if LINUX_VER_CODE > KERNEL_VERSION_NUM(2,6,30)
		parse_mac(argv[optind-1], macinfo);
		if (invert)
			macinfo->invert = 1;
#else
		parse_mac(argv[optind-1], &macinfo->srcaddr);
		if (invert)
			macinfo->flags |= MAC_SRC_INV;
		macinfo->flags |= MAC_SRC;
#endif
		*flags = 1;
		break;

	default:
		return 0;
	}

	return 1;
}

#if HAS_IPTABLES_SAVE || HAS_IPTABLES_PRINT
static void print_mac(const unsigned char macaddress[ETH_ALEN])
{
	unsigned int i;

	printf("%02X", macaddress[0]);
	for (i = 1; i < ETH_ALEN; i++)
		printf(":%02X", macaddress[i]);
	printf(" ");
}
#endif

static void mac_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "You must specify `--mac-source'");
}

#if HAS_IPTABLES_PRINT
static void
mac_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_mac_info *info = (void *)match->data;
	printf("MAC ");

	/* 2010-0827 mook:
	 *   Sync with netfilter xt_mac extension. */
#if LINUX_VER_CODE > KERNEL_VERSION_NUM(2,6,30)
	if (info->invert)
		printf("! ");
	print_mac(info->srcaddr);
#else
	if (info->flags & MAC_SRC_INV)
		printf("! ");
	print_mac(info->srcaddr.macaddr);
#endif
}
#endif

#if HAS_IPTABLES_SAVE
static void mac_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_mac_info *info = (void *)match->data;

	/* 2010-0827 mook:
	 *   Sync with netfilter xt_mac extension. */
#if LINUX_VER_CODE > KERNEL_VERSION_NUM(2,6,30)
	if (info->invert)
		printf("! ");
	printf("--mac-source ");
	print_mac(info->srcaddr);
#else
	if (info->flags & MAC_SRC_INV)
		printf("! ");

	printf("--mac-source ");
	print_mac(info->srcaddr.macaddr);
#endif
}
#endif

static struct xtables_match mac_match = {
	.family		= NFPROTO_IPV4,
 	.name		= "mac",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_mac_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_mac_info)),
#if HAS_IPTABLES_HELP
	.help		= mac_help,
#else
	.help		= NULL,
#endif
	.parse		= mac_parse,
	.final_check	= mac_check,
#if HAS_IPTABLES_PRINT
	.print		= mac_print,
#else
	.print		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save		= mac_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= mac_opts,
};

static struct xtables_match mac_match6 = {
	.family		= NFPROTO_IPV6,
 	.name		= "mac",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_mac_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_mac_info)),
#if HAS_IPTABLES_HELP
	.help		= mac_help,
#else
	.help		= NULL,
#endif
	.parse		= mac_parse,
	.final_check	= mac_check,
#if HAS_IPTABLES_PRINT
	.print		= mac_print,
#else
	.print		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save		= mac_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= mac_opts,
};

void _init(void)
{
	xtables_register_match(&mac_match);
	xtables_register_match(&mac_match6);
}
