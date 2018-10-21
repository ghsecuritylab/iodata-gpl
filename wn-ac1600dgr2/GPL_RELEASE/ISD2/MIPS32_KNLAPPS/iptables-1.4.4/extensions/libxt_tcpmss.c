/* Shared library add-on to iptables to add tcp MSS matching support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/xt_tcpmss.h>

#if HAS_IPTABLES_HELP
static void tcpmss_help(void)
{
	printf(
"tcpmss match options:\n"
"[!] --mss value[:value]	Match TCP MSS range.\n"
"				(only valid for TCP SYN or SYN/ACK packets)\n");
}
#endif

static const struct option tcpmss_opts[] = {
	{ "mss", 1, NULL, '1' },
	{ .name = NULL }
};

static u_int16_t
parse_tcp_mssvalue(const char *mssvalue)
{
	unsigned int mssvaluenum;

	if (xtables_strtoui(mssvalue, NULL, &mssvaluenum, 0, UINT16_MAX))
		return mssvaluenum;

	xtables_error(PARAMETER_PROBLEM,
		   "Invalid mss `%s' specified", mssvalue);
}

static void
parse_tcp_mssvalues(const char *mssvaluestring,
		    u_int16_t *mss_min, u_int16_t *mss_max)
{
	char *buffer;
	char *cp;

	buffer = strdup(mssvaluestring);
	if ((cp = strchr(buffer, ':')) == NULL)
		*mss_min = *mss_max = parse_tcp_mssvalue(buffer);
	else {
		*cp = '\0';
		cp++;

		*mss_min = buffer[0] ? parse_tcp_mssvalue(buffer) : 0;
		*mss_max = cp[0] ? parse_tcp_mssvalue(cp) : 0xFFFF;
	}
	free(buffer);
}

static int
tcpmss_parse(int c, char **argv, int invert, unsigned int *flags,
             const void *entry, struct xt_entry_match **match)
{
	struct xt_tcpmss_match_info *mssinfo =
		(struct xt_tcpmss_match_info *)(*match)->data;

	switch (c) {
	case '1':
		if (*flags)
			xtables_error(PARAMETER_PROBLEM,
				   "Only one `--mss' allowed");
		xtables_check_inverse(optarg, &invert, &optind, 0);
		parse_tcp_mssvalues(argv[optind-1],
				    &mssinfo->mss_min, &mssinfo->mss_max);
		if (invert)
			mssinfo->invert = 1;
		*flags = 1;
		break;
	default:
		return 0;
	}
	return 1;
}

static void tcpmss_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "tcpmss match: You must specify `--mss'");
}

#if HAS_IPTABLES_PRINT
static void
tcpmss_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_tcpmss_match_info *info = (void *)match->data;

	printf("tcpmss match %s", info->invert ? "!" : "");
	if (info->mss_min == info->mss_max)
		printf("%u ", info->mss_min);
	else
		printf("%u:%u ", info->mss_min, info->mss_max);
}
#endif

#if HAS_IPTABLES_SAVE
static void tcpmss_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_tcpmss_match_info *info = (void *)match->data;

	printf("%s--mss ", info->invert ? "! " : "");
	if (info->mss_min == info->mss_max)
		printf("%u ", info->mss_min);
	else
		printf("%u:%u ", info->mss_min, info->mss_max);
}
#endif

static struct xtables_match tcpmss_match = {
	.family		= NFPROTO_IPV4,
	.name		= "tcpmss",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_tcpmss_match_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_tcpmss_match_info)),
#if HAS_IPTABLES_HELP
	.help		= tcpmss_help,
#else
	.help		= NULL,
#endif
	.parse		= tcpmss_parse,
	.final_check	= tcpmss_check,
#if HAS_IPTABLES_PRINT
	.print		= tcpmss_print,
#else
	.print		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save		= tcpmss_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= tcpmss_opts,
};

static struct xtables_match tcpmss_match6 = {
	.family		= NFPROTO_IPV6,
	.name		= "tcpmss",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_tcpmss_match_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_tcpmss_match_info)),
#if HAS_IPTABLES_HELP
	.help		= tcpmss_help,
#else
	.help		= NULL,
#endif
	.parse		= tcpmss_parse,
	.final_check	= tcpmss_check,
#if HAS_IPTABLES_PRINT
	.print		= tcpmss_print,
#else
	.print		= NULL,
#endif
#if HAS_IPTABLES_SAVE
	.save		= tcpmss_save,
#else
	.save		= NULL,
#endif
	.extra_opts	= tcpmss_opts,
};

void _init(void)
{
	xtables_register_match(&tcpmss_match);
	xtables_register_match(&tcpmss_match6);
}
