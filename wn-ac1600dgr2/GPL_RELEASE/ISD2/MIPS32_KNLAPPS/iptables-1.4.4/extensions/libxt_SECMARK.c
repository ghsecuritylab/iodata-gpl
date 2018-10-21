/*
 * Shared library add-on to iptables to add SECMARK target support.
 *
 * Based on the MARK target.
 *
 * Copyright (C) 2006 Red Hat, Inc., James Morris <jmorris@redhat.com>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <xtables.h>
#include <linux/netfilter/xt_SECMARK.h>

#define PFX "SECMARK target: "

#if HAS_IPTABLES_HELP
static void SECMARK_help(void)
{
	printf(
"SECMARK target options:\n"
"  --selctx value                     Set the SELinux security context\n");
}
#endif

static const struct option SECMARK_opts[] = {
	{ "selctx", 1, NULL, '1' },
	{ .name = NULL }
};

static int SECMARK_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_target **target)
{
	struct xt_secmark_target_info *info =
		(struct xt_secmark_target_info*)(*target)->data;

	switch (c) {
	case '1':
		if (*flags & SECMARK_MODE_SEL)
			xtables_error(PARAMETER_PROBLEM, PFX
				   "Can't specify --selctx twice");
		info->mode = SECMARK_MODE_SEL;

		if (strlen(optarg) > SECMARK_SELCTX_MAX-1)
			xtables_error(PARAMETER_PROBLEM, PFX
				   "Maximum length %u exceeded by --selctx"
				   " parameter (%zu)",
				   SECMARK_SELCTX_MAX-1, strlen(optarg));

		strcpy(info->u.sel.selctx, optarg);
		*flags |= SECMARK_MODE_SEL;
		break;
	default:
		return 0;
	}

	return 1;
}

static void SECMARK_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM, PFX "parameter required");
}

#if HAS_IPTABLES_SAVE || HAS_IPTABLES_PRINT
static void print_secmark(const struct xt_secmark_target_info *info)
{
	switch (info->mode) {
	case SECMARK_MODE_SEL:
		printf("selctx %s ", info->u.sel.selctx);\
		break;
	
	default:
		xtables_error(OTHER_PROBLEM, PFX "invalid mode %hhu\n", info->mode);
	}
}
#endif

#if HAS_IPTABLES_PRINT
static void SECMARK_print(const void *ip, const struct xt_entry_target *target,
                          int numeric)
{
	const struct xt_secmark_target_info *info =
		(struct xt_secmark_target_info*)(target)->data;

	printf("SECMARK ");
	print_secmark(info);
}
#endif

#if HAS_IPTABLES_SAVE
static void SECMARK_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_secmark_target_info *info =
		(struct xt_secmark_target_info*)target->data;

	printf("--");
	print_secmark(info);
}
#endif

static struct xtables_target secmark_target = {
	.family		= NFPROTO_UNSPEC,
	.name		= "SECMARK",
	.version	= XTABLES_VERSION,
	.revision	= 0,
	.size		= XT_ALIGN(sizeof(struct xt_secmark_target_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_secmark_target_info)),
#if HAS_IPTABLES_HELP
	.help		= SECMARK_help,
#else
	.help		= NULL,
#endif
	.parse		= SECMARK_parse,
	.final_check	= SECMARK_check,
#if HAS_IPTABLES_PRINT
	.print		= SECMARK_print,
#else
	.print		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save		= SECMARK_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= SECMARK_opts,
};

void _init(void)
{
	xtables_register_target(&secmark_target);
}
