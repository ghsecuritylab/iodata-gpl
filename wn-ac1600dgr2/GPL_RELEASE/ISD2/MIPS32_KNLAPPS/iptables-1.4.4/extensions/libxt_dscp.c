/* Shared library add-on to iptables for DSCP
 *
 * (C) 2002 by Harald Welte <laforge@gnumonks.org>
 *
 * This program is distributed under the terms of GNU GPL v2, 1991
 *
 * libipt_dscp.c borrowed heavily from libipt_tos.c
 *
 * --class support added by Iain Barnes
 * 
 * For a list of DSCP codepoints see 
 * http://www.iana.org/assignments/dscp-registry
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_dscp.h>

/* This is evil, but it's my code - HW*/
#include "dscp_helper.c"

#if HAS_IPTABLES_HELP
static void dscp_help(void)
{
	printf(
"dscp match options\n"
"[!] --dscp value		Match DSCP codepoint with numerical value\n"
"  		                This value can be in decimal (ex: 32)\n"
"               		or in hex (ex: 0x20)\n"
"[!] --dscp-class name		Match the DiffServ class. This value may\n"
"				be any of the BE,EF, AFxx or CSx classes\n"
"\n"
"				These two options are mutually exclusive !\n");
}
#endif

static const struct option dscp_opts[] = {
	{ "dscp", 1, NULL, 'F' },
	{ "dscp-class", 1, NULL, 'G' },
	{ .name = NULL }
};

static void
parse_dscp(const char *s, struct xt_dscp_info *dinfo)
{
	unsigned int dscp;
       
	if (!xtables_strtoui(s, NULL, &dscp, 0, UINT8_MAX))
		xtables_error(PARAMETER_PROBLEM,
			   "Invalid dscp `%s'\n", s);

	if (dscp > XT_DSCP_MAX)
		xtables_error(PARAMETER_PROBLEM,
			   "DSCP `%d` out of range\n", dscp);

	dinfo->dscp = dscp;
}


static void
parse_class(const char *s, struct xt_dscp_info *dinfo)
{
	unsigned int dscp = class_to_dscp(s);

	/* Assign the value */
	dinfo->dscp = dscp;
}


static int
dscp_parse(int c, char **argv, int invert, unsigned int *flags,
           const void *entry, struct xt_entry_match **match)
{
	struct xt_dscp_info *dinfo
		= (struct xt_dscp_info *)(*match)->data;

	switch (c) {
	case 'F':
		if (*flags)
			xtables_error(PARAMETER_PROBLEM,
			           "DSCP match: Only use --dscp ONCE!");
		xtables_check_inverse(optarg, &invert, &optind, 0);
		parse_dscp(argv[optind-1], dinfo);
		if (invert)
			dinfo->invert = 1;
		*flags = 1;
		break;

	case 'G':
		if (*flags)
			xtables_error(PARAMETER_PROBLEM,
					"DSCP match: Only use --dscp-class ONCE!");
		xtables_check_inverse(optarg, &invert, &optind, 0);
		parse_class(argv[optind - 1], dinfo);
		if (invert)
			dinfo->invert = 1;
		*flags = 1;
		break;

	default:
		return 0;
	}

	return 1;
}

static void dscp_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
		           "DSCP match: Parameter --dscp is required");
}

#if HAS_IPTABLES_PRINT
static void
dscp_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_dscp_info *dinfo =
		(const struct xt_dscp_info *)match->data;
	printf("DSCP match %s0x%02x", dinfo->invert ? "!" : "", dinfo->dscp);
}
#endif

#if HAS_IPTABLES_SAVE
static void dscp_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_dscp_info *dinfo =
		(const struct xt_dscp_info *)match->data;

	printf("%s--dscp 0x%02x ", dinfo->invert ? "! " : "", dinfo->dscp);
}
#endif

static struct xtables_match dscp_match = {
	.family		= NFPROTO_IPV4,
	.name 		= "dscp",
	.version 	= XTABLES_VERSION,
	.size 		= XT_ALIGN(sizeof(struct xt_dscp_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_dscp_info)),
#if HAS_IPTABLES_HELP
	.help		= dscp_help,
#else
	.help		= NULL,
#endif
	.parse		= dscp_parse,
	.final_check	= dscp_check,
#if HAS_IPTABLES_PRINT
	.print		= dscp_print,
#else
	.print		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save		= dscp_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= dscp_opts,
};

static struct xtables_match dscp_match6 = {
	.family		= NFPROTO_IPV6,
	.name 		= "dscp",
	.version 	= XTABLES_VERSION,
	.size 		= XT_ALIGN(sizeof(struct xt_dscp_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_dscp_info)),
#if HAS_IPTABLES_HELP
	.help		= dscp_help,
#else
	.help		= NULL,
#endif
	.parse		= dscp_parse,
	.final_check	= dscp_check,
#if HAS_IPTABLES_PRINT
	.print		= dscp_print,
#else
	.print		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save		= dscp_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= dscp_opts,
};

void _init(void)
{
	xtables_register_match(&dscp_match);
	xtables_register_match(&dscp_match6);
}
