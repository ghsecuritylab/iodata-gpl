#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

#include "dhcpd.h"
#include "leases.h"
#include "libbb_udhcp.h"

#define REMAINING 0
#define ABSOLUTE 1
#define SIMPLE_REMAINING 2
#define EXPIRED_UNTIL 3
#define RESOLVE_HOSTNAME 4
#define TWO_WEEKS 1209600


#ifndef IN_BUSYBOX
static void __attribute__ ((noreturn)) show_usage(void)
{
	printf(
		  "Usage: dumpleases -f <file> -[r|a]\n\n"
		  "  -f, --file=FILENAME             Leases file to load\n"
		  "  -r, --remaining                 Interpret lease times as time remaing\n"
		  "  -t, --simple remaining          Interpret simple lease times as time remaing\n"
		  "  -d, --dump                      Dump lease type: ignore Expired\n"
		  "  -a, --absolute                  Interpret lease times as expire time\n"
#if HAS_DHCP_IP2HOSTNAME
		  "  -i, --resolve                   Resolve ip to hostname\n"
#endif
		  "  -p, --print                     print client hostname\n"
		  "  -T, --newline                   Interpret simple lease times as time remaing and split them in different line\n"
		  "  -e, --expire                    Interpret lease times as exact expired time\n");
	exit(0);
}
#else
	#define show_usage bb_show_usage
#endif

#ifdef IN_BUSYBOX
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
    int printHostname = 0;
    int newline = 0;
	struct dhcpOfferedAddr lease;
	struct in_addr addr;
	time_t seconds;
	struct tm *timeinfo;
	char buf[30]={0};
#if HAS_DHCP_IP2HOSTNAME
	char ip_str[16];
#endif
	static const struct option options[] = {
		{"absolute", 0, 0, 'a'},
		{"remaining", 0, 0, 'r'},
		{"dump", 0, 0, 'd'},
		{"simple remaining", 0, 0, 't'},
		{"file", 1, 0, 'f'},
#if HAS_DHCP_IP2HOSTNAME
		{"ip2host", 1, 0, 'm'},		
#endif
		{"print", 0, 0, 'p'},
		{"newline", 0, 0, 'T'},
		{"expire", 0, 0, 'e'},
		{0, 0, 0, 0}
	};

	while(1)
	{
		int option_index = 0;
		c = getopt_long(argc, argv, "ardtepTf:i:", options, &option_index);
		if(c == -1)	break;

		switch(c)
		{
		case 'a': mode = ABSOLUTE; break;
		case 'r': mode = REMAINING; break;
		case 't': mode = SIMPLE_REMAINING; break;
		case 'e': mode = EXPIRED_UNTIL; break;
		case 'd':
			dump = 1;
		case 'f':
			file = optarg;
			break;
#if HAS_DHCP_IP2HOSTNAME
		case 'i':
            strcpy(ip_str,optarg);
			mode = RESOLVE_HOSTNAME;
			break;		
#endif
		case 'p':
			printHostname = 1;
			break;
        case 'T':
            mode = SIMPLE_REMAINING;
			newline = 1;
			break;	
		default:
			show_usage();
		}
	}

	fp = xfopen(file, "r");

	/*Fix the issue that  if /etc/udhcpd.leases doesn't exist, the system may still determine that it exists*/
    if(fp)
    {
	//printf("Mac Address       IP-Address      Expires %s\n", mode == REMAINING ? "in" : "at");
	/*     "00:00:00:00:00:00 255.255.255.255 Wed Jun 30 21:49:08 1993" */
	while(fread(&lease, sizeof(lease), 1, fp))
	{

#if 1 //cfho 2007-1129
		if(lease.chaddr[0]==0 && lease.chaddr[1]==0 && lease.chaddr[2]==0 &&
		   lease.chaddr[3]==0 && lease.chaddr[4]==0 && lease.chaddr[5]==0 )
		{
			continue;
		}
#endif
		expires = ntohl(lease.expires);
		if(!expires)
		{
			if(1==dump)	/* skip*/
				continue;
		}
#if HAS_DHCP_IP2HOSTNAME
		if(mode == RESOLVE_HOSTNAME)
		{
            addr.s_addr = lease.yiaddr;
			if(strcmp(inet_ntoa(addr),ip_str) == 0)
			{
#if HAS_DHCP_HOST_FUNCTION && !FOR_EG_ESR_TR
			//printf(" %-20s&", lease.hostname);
			if(0==strlen(lease.hostname))
				printf("---\n");
			else
				printf("%s\n", lease.hostname);
#endif
				break;
			}
			else
			{
				continue;
			}
		}
#endif
		for(i = 0; i < 6; i++)
		{
			printf("%02X", lease.chaddr[i]);
			if(i != 5) printf(":");
		}
		printf("&");
		addr.s_addr = lease.yiaddr;
		printf("%s&", inet_ntoa(addr));
#if HAS_DHCP_HOST_FUNCTION && !FOR_EG_ESR_TR
		printHostname = 1;
#endif
        if(printHostname)
        {
    		if(0==strlen(lease.hostname))
    			printf(" &");
    		else
    			printf("%s&", lease.hostname);
        }

#if HAS_DHCPD_IF_INFOMATION_FUNCTION && !FOR_EG_ESR_TR
		printf(" %-10s&", lease.ifName);
#endif
		expires = ntohl(lease.expires);
		printf(" ");
		if(mode == REMAINING)
		{
			if(!expires) printf("Expired&\n");
#if 0
			else
			{
				if(expires > 60*60*24)
				{
					printf("%ld days, ", expires / (60*60*24));
					expires %= 60*60*24;
				}
				if(expires > 60*60)
				{
					printf("%ld hours, ", expires / (60*60));
					expires %= 60*60;
				}
				if(expires > 60)
				{
					printf("%ld minutes, ", expires / 60);
					expires %= 60;
				}
				printf("%ld seconds&\n", expires);
			}
#else //from busy box 1.7x
			else
			{
				unsigned d, h, m;
				d = expires / (24*60*60); expires %= (24*60*60);
				h = expires / (60*60); expires %= (60*60);
				m = expires / 60; expires %= 60;
				if(d<3000)
				{
					printf("%u day%s ", d,(d>1)?"s":" ");
					printf("%02u:%02u:%02u&\n", h, m, (unsigned)expires);
				}
				else
					printf("Forever&\n");
			}
#endif
		}
		else if(mode == SIMPLE_REMAINING)
		{
			/* jerry: for TR-069 agent */
			if (!expires) 
				printf("0&%s",(newline)?"\n":"");
			else if(expires > TWO_WEEKS) // Forever
				printf("-1&%s",(newline)?"\n":"");
			else 
				printf("%ld&%s", expires,(newline)?"\n":"");
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
		}
		else if(mode == EXPIRED_UNTIL) 
		{
			seconds = time(NULL);
			seconds += expires;
			timeinfo = localtime(&seconds);
			mktime(timeinfo);
			sprintf(buf, "%s", asctime(timeinfo));
			buf[strlen(buf)-1]=0; /* remove \n */
			printf("%s&\n", buf);
		}
		else printf("%s", ctime(&expires));
	}
	fclose(fp);
    }

	return 0;
}
