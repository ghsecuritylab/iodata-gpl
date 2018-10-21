/* Shared library add-on to iptables to add comment match support.
 *
 * ChangeLog
 *     2003-05-13: Brad Fisher <brad@info-link.net>
 *         Initial comment match
 *     2004-05-12: Brad Fisher <brad@info-link.net>
 *         Port to patch-o-matic-ng
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/xt_comment.h>

#if HAS_IPTABLES_HELP
static void comment_help(void)
{
	printf(
		"comment match options:\n"
		"--comment COMMENT             Attach a comment to a rule\n");
}
#endif

static const struct option comment_opts[] = {
	{ "comment", 1, NULL, '1' },
	{ .name = NULL }
};

static void
parse_comment(const char *s, struct xt_comment_info *info)
{	
	int slen = strlen(s);

	if (slen >= XT_MAX_COMMENT_LEN) {
		xtables_error(PARAMETER_PROBLEM,
			"COMMENT must be shorter than %i characters", XT_MAX_COMMENT_LEN);
	}
	strcpy((char *)info->comment, s);
}

static int
comment_parse(int c, char **argv, int invert, unsigned int *flags,
              const void *entry, struct xt_entry_match **match)
{
	struct xt_comment_info *commentinfo = (struct xt_comment_info *)(*match)->data;

	switch (c) {
	case '1':
		xtables_check_inverse(argv[optind-1], &invert, &optind, 0);
		if (invert) {
			xtables_error(PARAMETER_PROBLEM,
					"Sorry, you can't have an inverted comment");
		}
		parse_comment(argv[optind-1], commentinfo);
		*flags = 1;
		break;

	default:
		return 0;
	}
	return 1;
}

static void comment_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "COMMENT match: You must specify `--comment'");
}

#if HAS_IPTABLES_PRINT
static void
comment_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	struct xt_comment_info *commentinfo = (void *)match->data;

	commentinfo->comment[XT_MAX_COMMENT_LEN-1] = '\0';
	printf("/* %s */ ", commentinfo->comment);
}
#endif

/* Saves the union ipt_matchinfo in parsable form to stdout. */
#if HAS_IPTABLES_SAVE
static void
comment_save(const void *ip, const struct xt_entry_match *match)
{
	struct xt_comment_info *commentinfo = (void *)match->data;

	commentinfo->comment[XT_MAX_COMMENT_LEN-1] = '\0';
	printf("--comment ");
	xtables_save_string((const char *)commentinfo->comment);
}
#endif

static struct xtables_match comment_match = {
	.family		= NFPROTO_IPV4,
	.name		= "comment",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_comment_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_comment_info)),
#if HAS_IPTABLES_HELP
	.help		= comment_help,
#else
	.help		= NULL,
#endif
	.parse		= comment_parse,
	.final_check	= comment_check,
#if HAS_IPTABLES_PRINT
	.print 		= comment_print,
#else
	.print 		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save 		= comment_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= comment_opts,
};

static struct xtables_match comment_match6 = {
	.family		= NFPROTO_IPV6,
	.name		= "comment",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_comment_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_comment_info)),
#if HAS_IPTABLES_HELP
	.help		= comment_help,
#else
	.help		= NULL,
#endif
	.parse		= comment_parse,
	.final_check	= comment_check,
#if HAS_IPTABLES_PRINT
	.print 		= comment_print,
#else
	.print 		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save 		= comment_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= comment_opts,
};

void _init(void)
{
	xtables_register_match(&comment_match);
	xtables_register_match(&comment_match6);
}
