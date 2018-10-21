/* Shared library add-on to iptables to add related packet matching support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/xt_helper.h>

#if HAS_IPTABLES_HELP
static void helper_help(void)
{
	printf(
"helper match options:\n"
"[!] --helper string        Match helper identified by string\n");
}
#endif

static const struct option helper_opts[] = {
	{ "helper", 1, NULL, '1' },
	{ .name = NULL }
};

static int
helper_parse(int c, char **argv, int invert, unsigned int *flags,
             const void *entry, struct xt_entry_match **match)
{
	struct xt_helper_info *info = (struct xt_helper_info *)(*match)->data;

	switch (c) {
	case '1':
		if (*flags)
			xtables_error(PARAMETER_PROBLEM,
					"helper match: Only use --helper ONCE!");
		xtables_check_inverse(optarg, &invert, &invert, 0);
		strncpy(info->name, optarg, 29);
		info->name[29] = '\0';
		if (invert)
			info->invert = 1;
		*flags = 1;
		break;

	default:
		return 0;
	}
	return 1;
}

static void helper_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "helper match: You must specify `--helper'");
}

#if HAS_IPTABLES_PRINT
static void
helper_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_helper_info *info = (const void *)match->data;

	printf("helper match %s\"%s\" ", info->invert ? "! " : "", info->name);
}
#endif

#if HAS_IPTABLES_SAVE
static void helper_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_helper_info *info = (const void *)match->data;

	printf("%s--helper ",info->invert ? "! " : "");
	xtables_save_string(info->name);
}
#endif

static struct xtables_match helper_match = {
	.family		= NFPROTO_IPV4,
	.name		= "helper",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_helper_info)),
#if HAS_IPTABLES_HELP
	.help		= helper_help,
#else
	.help		= NULL,
#endif
	.parse		= helper_parse,
	.final_check	= helper_check,
#if HAS_IPTABLES_PRINT
	.print		= helper_print,
#else
	.print		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save		= helper_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= helper_opts,
};

static struct xtables_match helper_match6 = {
	.family		= NFPROTO_IPV6,
	.name		= "helper",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_helper_info)),
#if HAS_IPTABLES_HELP
	.help		= helper_help,
#else
	.help		= NULL,
#endif
	.parse		= helper_parse,
	.final_check	= helper_check,
#if HAS_IPTABLES_PRINT
	.print		= helper_print,
#else
	.print		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save		= helper_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= helper_opts,
};

void _init(void)
{
	xtables_register_match(&helper_match);
	xtables_register_match(&helper_match6);
}
