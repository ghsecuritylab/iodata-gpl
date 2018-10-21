/* Shared library add-on to iptables to add TRACE target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/x_tables.h>

#if HAS_IPTABLES_HELP
static void TRACE_help(void)
{
	printf("TRACE target takes no options\n");
}
#endif

static int TRACE_parse(int c, char **argv, int invert, unsigned int *flags,
                       const void *entry, struct xt_entry_target **target)
{
	return 0;
}

static struct xtables_target trace_target = {
	.family		= NFPROTO_UNSPEC,
	.name		= "TRACE",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(0),
	.userspacesize	= XT_ALIGN(0),
#if HAS_IPTABLES_HELP
	.help		= TRACE_help,
#else
	.help		= NULL,
#endif
	.parse		= TRACE_parse,
};

void _init(void)
{
	xtables_register_target(&trace_target);
}
