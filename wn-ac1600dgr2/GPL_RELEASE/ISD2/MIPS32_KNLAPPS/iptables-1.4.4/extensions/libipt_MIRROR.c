/* Shared library add-on to iptables to add MIRROR target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>

#if HAS_IPTABLES_HELP
static void MIRROR_help(void)
{
	printf("MIRROR target takes no options\n");
}
#endif

static int MIRROR_parse(int c, char **argv, int invert, unsigned int *flags,
                        const void *entry, struct xt_entry_target **target)
{
	return 0;
}

static struct xtables_target mirror_tg_reg = {
	.name		= "MIRROR",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV4,
	.size		= XT_ALIGN(0),
	.userspacesize	= XT_ALIGN(0),
#if HAS_IPTABLES_HELP
 	.help		= MIRROR_help,
#else
	.help		= NULL,
#endif
 	.parse		= MIRROR_parse,
	.print		= NULL,
	.save		= NULL,
};

void _init(void)
{
	xtables_register_target(&mirror_tg_reg);
}
