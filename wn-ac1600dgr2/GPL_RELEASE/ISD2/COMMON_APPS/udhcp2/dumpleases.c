/* vi: set sw=4 ts=4: */
/*
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 */
#include <getopt.h>

#include "common.h"
#include "dhcpd.h"

#define REMAINING 0
#define ABSOLUTE 1
#define SIMPLE_REMAINING 2

#define TWO_WEEKS 1209600

#ifndef COMBINED_BINARY
static void __attribute__ ((noreturn)) show_usage(void)
{
	printf(
"Usage: dumpleases -f <file> -[r|a]\n\n"
"  -f, --file=FILENAME             Leases file to load\n"
"  -r, --remaining                 Interepret lease times as time remaing\n"
"  -t, --simple remaining          Interpret simple lease times as time remaing\n"
"  -d, --dump                      Dump lease type: ignore Expired\n"
"  -a, --absolute                  Interepret lease times as expire time\n");
	exit(0);
}
#else
#define show_usage bb_show_usage
#endif

#ifdef COMBINED_BINARY
int dumpleases_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	FILE *fp;
	int i, c, mode = REMAINING;
	long expires;
	const char *file = LEASES_FILE;
	int dump = 0;
	struct dhcpOfferedAddr lease;
	struct in_addr addr;

	static const struct option options[] = {
		{"absolute", 0, 0, 'a'},
		{"remaining", 0, 0, 'r'},
		{"dump", 0, 0, 'd'},
		{"simple remaining", 0, 0, 't'},
		{"file", 1, 0, 'f'},
		{0, 0, 0, 0}
	};

	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "ardtf:", options, &option_index);
		if (c == -1) break;

		switch (c) {
		case 'a': mode = ABSOLUTE; break;
		case 'r': mode = REMAINING; break;
		case 't': mode = SIMPLE_REMAINING; break;
		case 'd':
			dump = 1;
			break;
		case 'f':
			file = optarg;
			break;
		default:
			show_usage();
		}
	}

	fp = fopen(file, "r");

	//	printf("Mac Address       IP-Address      Expires %s\n", (opt & OPT_a) ? "at" : "in");
	/*     "00:00:00:00:00:00 255.255.255.255 Wed Jun 30 21:49:08 1993" */
	while (fread(&lease, sizeof(lease), 1, fp)) {
#if 1 //cfho 2007-1129
        if (lease.chaddr[0]==0 && lease.chaddr[1]==0 && lease.chaddr[2]==0 &&
            lease.chaddr[3]==0 && lease.chaddr[4]==0 && lease.chaddr[5]==0 )
        {
            continue;
        }
#endif
		expires = ntohl(lease.expires);
		if (!expires) 
		{
			if(1==dump) /* skip*/
				continue;
		}
			
		for (i = 0; i < 6; i++) {
			printf("%02X", lease.chaddr[i]);
			if (i != 5) printf(":");
		}
		printf("&");
		addr.s_addr = lease.yiaddr;
		printf("%s&", inet_ntoa(addr));
#if HAS_DHCP_HOST_FUNCTION && !FOR_EG_ESR_TR
		//printf(" %-20s&", lease.hostname);
		if(0==strlen(lease.hostname))
		    printf(" &");
		else
		    printf("%s&", lease.hostname);
#endif
#if HAS_DHCPD_IF_INFOMATION_FUNCTION && !FOR_EG_ESR_TR
		printf(" %s& ", lease.ifName);
#endif
		expires = ntohl(lease.expires);
		printf(" ");
		if (mode == REMAINING) {
			if (!expires) printf("Expired&\n");
            else {
                unsigned d, h, m;
                d = expires / (24*60*60); expires %= (24*60*60);
                h = expires / (60*60); expires %= (60*60);
                m = expires / 60; expires %= 60;
                if (d<3000) {printf("%u day%s ", d,(d>1)?"s":" ");
                printf("%02u:%02u:%02u&\n", h, m, (unsigned)expires);
                }
                else
                printf("Forever&\n");
            }
		}
		else if(mode == SIMPLE_REMAINING)
		{
			/* jerry: for TR-069 agent */
			if (!expires) 
				printf("0&");
			else if(expires > TWO_WEEKS) // Forever
				printf("-1&");
			else 
				printf("%ld&", expires);
#if HAS_DHCP_HOST_FUNCTION && FOR_EG_ESR_TR
			//printf(" %-20s&", lease.hostname);
			if(0==strlen(lease.hostname))
				printf(" &");
			else
				printf("%s&", lease.hostname);
#endif
#if HAS_DHCPD_IF_INFOMATION_FUNCTION && FOR_EG_ESR_TR
			printf(" %-10s&\n", lease.ifName);
#endif
		} else printf("%s", ctime(&expires));
	}
	fclose(fp);

	return 0;
}
