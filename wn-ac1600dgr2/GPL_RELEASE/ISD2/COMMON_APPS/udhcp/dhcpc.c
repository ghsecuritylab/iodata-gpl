/* dhcpc.c
 *
 * udhcp DHCP client
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/time.h>
#include <sys/file.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#include "dhcpd.h"
#include "dhcpc.h"
#include "options.h"
#include "clientpacket.h"
#include "clientsocket.h"
#include "script.h"
#include "socket.h"
#include "common.h"
#include "signalpipe.h"

#include <gconfig.h>
#include "checkLink.h"
#include <math.h>

#if HAS_SYSTEM_LOG
#include <syslog.h>
#include <stdarg.h>
#endif

static int state;
static unsigned long requested_ip; /* = 0 */
static unsigned long server_addr;
static unsigned long timeout;
static int packet_num; /* = 0 */
static int fd = -1;

/* Fix Local 4th IP: xxx.xxx.xxx.4thIP */
int fixLocal4thIP = 0;

#define LISTEN_NONE 0
#define LISTEN_KERNEL 1
#define LISTEN_RAW 2

#if HAS_NETBIOS_FUNCTION
#define NETBIOS_WINS_IP_1_LEN            4
#define NETBIOS_WINS_IP_1_AND_2_LEN      8
#define DHCP_NETBIOS_LEARN_FROM_WAN_FILE "/tmp/netbios_info_from_wan"
#endif

#define DHCP_LOG_FILE "/tmp/dhcpc.lease"

static int listen_mode;
int key_index=0;

struct client_config_t client_config = {
	/* Default options. */
	.abort_if_no_lease = 0,
	.foreground = 0,
	.quit_after_lease = 0,
	.background_if_no_lease = 0,
	.interface = "eth0",
	.pidfile = NULL,
	.script = DEFAULT_SCRIPT,
	.clientid = NULL,
	.vendorclass = NULL,
	.hostname = NULL,
	.fqdn = NULL,
	.ifindex = 0,
	.arp = "\0\0\0\0\0\0",		/* appease gcc-3.0 */
#if SUPPORT_IPV6_6RD
    .option_6rd_enable = 0,     /* support for enable 6rd DHCPv4 Option (212) */
#endif
	.classless_static_route_enable = 1, /* support for enable classless static route (249) */
    .bootp_broadcast_flag = 0,
	// 2013-10-31 Norkay, sending packet's interface
	.intf_send = NULL,
	.ifidx_send = 0,
	.arp_send = "\0\0\0\0\0\0"
};

#ifndef IN_BUSYBOX
static void __attribute__ ((noreturn)) show_usage(void)
{
	printf(
"Usage: udhcpc [OPTIONS]\n\n"
"  -c, --clientid=CLIENTID         Set client identifier - type is first char\n"
"  -C, --clientid-none             Suppress default client identifier\n"
"  -V, --vendorclass=CLASSID       Set vendor class identifier\n"
"  -H, --hostname=HOSTNAME         Client hostname\n"
"  -h                              Alias for -H\n"
"  -F, --fqdn=FQDN                 Client fully qualified domain name\n"
"  -f, --foreground                Do not fork after getting lease\n"
"  -b, --background                Fork to background if lease cannot be\n"
"                                  immediately negotiated.\n"
"  -i, --interface=INTERFACE       Interface to use (default: eth0)\n"
"  -n, --now                       Exit with failure if lease cannot be\n"
"                                  immediately negotiated.\n"
"  -p, --pidfile=file              Store process ID of daemon in file\n"
"  -q, --quit                      Quit after obtaining lease\n"
"  -r, --request=IP                IP address to request (default: none)\n"
"  -s, --script=file               Run file at dhcp events (default:\n"
"                                  " DEFAULT_SCRIPT ")\n"
"  -v, --version                   Display version\n"
#if SUPPORT_IPV6_6RD
"  --option212_on                  for option_6rd_enable\n"
#endif
"  --option249_on                  for classless_static_route_enable\n"
	);
	exit(0);
}
#else
#define show_usage bb_show_usage
extern void show_usage(void) __attribute__ ((noreturn));
#endif

#if HAS_SYSTEM_LOG
/*****************************************************************
* NAME: SystemLog
* ---------------------------------------------------------------
* FUNCTION:  
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
****************************************************************/
int SystemLog(char *format, ...)
{
    va_list ap;
    char SystemLogBuf[256];

    va_start(ap, format);
    vsnprintf(SystemLogBuf, sizeof(SystemLogBuf), format, ap);
    /* printf("----->%s\n", SystemLogBuf); */
    openlog( "systemd", LOG_NDELAY, LOG_LOCAL1);

    syslog(LOG_INFO, SystemLogBuf);
    closelog();

    return 0;
}
#endif

/* just a little helper */
static void change_mode(int new_mode)
{
	DEBUG(LOG_INFO, "entering %s listen mode",
		new_mode ? (new_mode == 1 ? "kernel" : "raw") : "none");
	if (fd >= 0) close(fd);
	fd = -1;
	listen_mode = new_mode;
}


/* perform a renew */
static void perform_renew(void)
{
	LOG(LOG_INFO, "Performing a DHCP renew");
	switch (state) {
	case BOUND:
		change_mode(LISTEN_KERNEL);
	case RENEWING:
	case REBINDING:
		state = RENEW_REQUESTED;
		break;
	case RENEW_REQUESTED: /* impatient are we? fine, square 1 */
		run_script(NULL, "deconfig");
	case REQUESTING:
	case RELEASED:
		change_mode(LISTEN_RAW);
		state = INIT_SELECTING;
		break;
	case INIT_SELECTING:
		break;
	}

	/* start things over */
	packet_num = 0;

	/* Kill any timeouts because the user wants this to hurry along */
	timeout = 0;
}


/* perform a release */
static void perform_release(void)
{
	char buffer[16];
	struct in_addr temp_addr;

//     if(state==RELEASED) return;
#if 1
	/* send release packet */
	if (state == BOUND || state == RENEWING || state == REBINDING) {
		temp_addr.s_addr = server_addr;
		sprintf(buffer, "%s", inet_ntoa(temp_addr));
		temp_addr.s_addr = requested_ip;
		LOG(LOG_INFO, "Unicasting a release of %s to %s",
				inet_ntoa(temp_addr), buffer);
		send_release(server_addr, requested_ip); /* unicast */
		run_script(NULL, "deconfig");
	}
#endif
	LOG(LOG_INFO, "Entering released state");

	change_mode(LISTEN_NONE);
	state = RELEASED;
	timeout = 0x7fffffff;
}


static void client_background(void)
{
	background(client_config.pidfile);
	client_config.foreground = 1; /* Do not fork again. */
	client_config.background_if_no_lease = 0;
}

#if HAS_GRAT_ARP
#include <net/if_arp.h>
struct gratArpMsg {
    /* Ethernet header */
    u_char   h_dest[6];         /* destination ether addr */
    u_char   h_source[6];           /* source ether addr */
    u_short  h_proto;           /* packet type ID field */

    /* ARP packet */
    uint16_t htype;             /* hardware type (must be ARPHRD_ETHER) */
    uint16_t ptype;             /* protocol type (must be ETH_P_IP) */
    uint8_t  hlen;              /* hardware address length (must be 6) */
    uint8_t  plen;              /* protocol address length (must be 4) */
    uint16_t operation;         /* ARP opcode */
    uint8_t  sHaddr[6];         /* sender's hardware address */
    uint8_t  sInaddr[4];            /* sender's IP address */
    uint8_t  tHaddr[6];         /* target's hardware address */
    uint8_t  tInaddr[4];            /* target's IP address */
    //uint8_t  pad[18];           /* pad for min. Ethernet payload (60 bytes) */
} __attribute__ ((packed));

int GratArp(uint32_t ip, uint8_t *mac, char *interface)
{

    //int timeout = 2;
    int     optval = 1;
    int s;          /* socket */
    //int rv = 1;         /* return value */
    struct sockaddr addr;       /* for interface name */
    struct gratArpMsg   arp;
    //fd_set      fdset;
    //struct timeval  tm;
    //time_t      prevTime;


    if ((s = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) == -1) {
        printf("Error: Could not open raw socket");
        return -1;
    }

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1) {
        printf("Error: Could not setsocketopt on raw socket");
        close(s);
        return -1;
    }

    /* send arp request */
    memset(&arp, 0, sizeof(arp));
    memcpy(arp.h_dest, MAC_BCAST_ADDR, 6);      /* MAC DA */
    memcpy(arp.h_source, mac, 6);           /* MAC SA */
    arp.h_proto = htons(ETH_P_ARP);         /* protocol type (Ethernet) */
    arp.htype = htons(ARPHRD_ETHER);        /* hardware type */
    arp.ptype = htons(ETH_P_IP);            /* protocol type (ARP message) */
    arp.hlen = 6;                   /* hardware address length */
    arp.plen = 4;                   /* protocol address length */
    arp.operation = htons(ARPOP_REQUEST);       /* ARP op code */
    memcpy(arp.sInaddr, &ip, sizeof(ip));       /* source IP address */
    memcpy(arp.sHaddr, mac, 6);         /* source hardware address */
    memcpy(arp.tInaddr, &ip, sizeof(ip));   /* target IP address */
    memset(&addr, 0, sizeof(addr));
    strcpy(addr.sa_data, interface);
    if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0)
    {
        close(s);
        return 0;
    }
    
    close(s);
   return 1;
}
#endif

#ifdef COMBINED_BINARY
int udhcpc_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	uint8_t *temp, *message;
	unsigned long t1 = 0, t2 = 0, xid = 0;
	unsigned long start = 0, lease;
	fd_set rfds;
	int retval;
	struct timeval tv;
	int c, len;
	struct dhcpMessage packet;
	struct in_addr temp_addr, rip_addr;
	long now;
	int max_fd;
	int sig;
	int no_clientid = 0;
	int link_status;
	int backoffTime=0;
	int wan_detect = 0;
	int vpn_call = -1;
	int clientAtLanPort = 0;
    int clientAtWLanPort = 0;
	int violenceTest = 0;
	int violenceCount = 0;
	char commands[50];
	int backoffForAWhile = 0;
	int notCheckMii = 0;
	int hasZeroConfig = 0;
#if HAS_NETBIOS_FUNCTION
	FILE *fp_NetBios = NULL;
	int isLearnFromWanEnable = 0;
	uint32_t option_wins_ip_len = 0, nodeType = 0, scope_len = 0;
	unsigned long wins_ip_1, wins_ip_2;
	char scope[64] = {0};
#endif

	static const struct option arg_options[] = {
		{"clientid",	required_argument,	0, 'c'},
		{"clientid-none", no_argument,		0, 'C'},
		{"vendorclass",	required_argument,	0, 'V'},
		{"foreground",	no_argument,		0, 'f'},
		{"background",	no_argument,		0, 'b'},
		{"hostname",	required_argument,	0, 'H'},
		{"hostname",	required_argument,	0, 'h'},
		{"fqdn",	required_argument,	0, 'F'},
		{"interface",	required_argument,	0, 'i'},
		{"now", 	no_argument,		0, 'n'},
		{"pidfile",	required_argument,	0, 'p'},
		{"quit",	no_argument,		0, 'q'},
		{"request",	required_argument,	0, 'r'},
		{"script",	required_argument,	0, 's'},
		{"wandetect",	no_argument,		0, 'w'},
		{"version",	no_argument,		0, 'v'},
		{"key",	required_argument,		0, 'k'},
		{"fix4thIp",	required_argument,		0, 'a'},
#if SUPPORT_IPV6_6RD
		{"option212_on",	no_argument,		0, 0},
#endif
		{"option249_on",	no_argument,		0, 0},
		{0, 0, 0, 0}
	};

	/* get options */
	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "c:CV:fbH:h:F:i:np:qr:s:w:t:k:vT:ZlAWLa:BS:", arg_options, &option_index);
		if (c == -1) break;

		switch (c) {
		case 0:
			if(strcmp("option249_on",arg_options[option_index].name)==0)
			{
				client_config.classless_static_route_enable = 1;
				break;
			}
#if SUPPORT_IPV6_6RD
            if(strcmp("option212_on",arg_options[option_index].name)==0)
            {
                client_config.option_6rd_enable = 1;
                break;
            }
#endif
		case 'c':
			if (no_clientid) show_usage();
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.clientid) free(client_config.clientid);
			client_config.clientid = xmalloc(len + 2);
			client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
			client_config.clientid[OPT_LEN] = len;
			client_config.clientid[OPT_DATA] = '\0';
			strncpy((char*)(client_config.clientid + OPT_DATA), optarg, len);
			break;
		case 'C':
			if (client_config.clientid) show_usage();
			no_clientid = 1;
			break;
		case 'V':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.vendorclass) free(client_config.vendorclass);
			client_config.vendorclass = xmalloc(len + 2);
			client_config.vendorclass[OPT_CODE] = DHCP_VENDOR;
			client_config.vendorclass[OPT_LEN] = len;
			strncpy((char*)(client_config.vendorclass + OPT_DATA), optarg, len);
			break;
		case 'f':
			client_config.foreground = 1;
			break;
		case 'b':
			client_config.background_if_no_lease = 1;
			break;
		case 'h':
		case 'H':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.hostname) free(client_config.hostname);
			client_config.hostname = xmalloc(len + 2);
			client_config.hostname[OPT_CODE] = DHCP_HOST_NAME;
			client_config.hostname[OPT_LEN] = len;
			strncpy((char*)(client_config.hostname + 2), optarg, len);
			break;
		case 'F':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.fqdn) free(client_config.fqdn);
			client_config.fqdn = xmalloc(len + 5);
			client_config.fqdn[OPT_CODE] = DHCP_FQDN;
			client_config.fqdn[OPT_LEN] = len + 3;
			/* Flags: 0000NEOS
			S: 1 => Client requests Server to update A RR in DNS as well as PTR
			O: 1 => Server indicates to client that DNS has been updated regardless
			E: 1 => Name data is DNS format, i.e. <4>host<6>domain<4>com<0> not "host.domain.com"
			N: 1 => Client requests Server to not update DNS
			*/
			client_config.fqdn[OPT_LEN + 1] = 0x1;
			client_config.fqdn[OPT_LEN + 2] = 0;
			client_config.fqdn[OPT_LEN + 3] = 0;
			strncpy((char*)(client_config.fqdn + 5), optarg, len);
			break;
		case 'i':
			client_config.interface =  optarg;
			break;
		case 'n':
			client_config.abort_if_no_lease = 1;
			break;
		case 'p':
			client_config.pidfile = optarg;
			break;
		case 'q':
			client_config.quit_after_lease = 1;
			break;
		case 'r':
			requested_ip = inet_addr(optarg);
			break;
		case 's':
			client_config.script = optarg;
			break;
		case 'w':
			wan_detect = 1;
			break;
		case 't':
			vpn_call = atoi(optarg);
			break;
		case 'v':
			printf("udhcpcd, version %s\n\n", VERSION);
			return 0;
			break;
		case 'T':  
			/* violence test: request a lot of IP from dhcpd.
               USAGE:  [-T Count] */
			notCheckMii = 1;
			violenceTest = atoi(optarg);
			if(violenceTest > 0xff || violenceTest <= 0) return 0;
			break;
		case 'l':
			clientAtLanPort = 1;
            /* lan udhcp client  */
			key_index = -1;
			break;
		case 'A':
			backoffForAWhile = 1;
			break;
		case 'W':
			notCheckMii = 1;
			break;
		case 'L':
			clientAtWLanPort = 1;
            break;
		case 'Z':
			hasZeroConfig = 1;
			break;
		case 'k':
		    key_index = atoi(optarg);
		    printf("udhcpcd, key =  %d\n\n", key_index);
			break;
		case 'a':  
			/* Fix Local 4th IP: xxx.xxx.xxx.4thIP
               USAGE:  [-a 4thIP] */
			fixLocal4thIP = atoi(optarg);
			break;
        case 'B':
            client_config.bootp_broadcast_flag = 1;
            break;
		case 'S':
            client_config.intf_send = optarg;
			break;
		default:
			show_usage();
		}
	}

	/* Start the log, sanitize fd's, and write a pid file */
	start_log_and_pid("udhcpc", client_config.pidfile);

	if (read_interface(client_config.interface, &client_config.ifindex,
			   NULL, client_config.arp) < 0)
		return 1;

	// 2013-10-31 Norkay, Get sending packet's interface ifindex
	if(client_config.intf_send != NULL)
	{
		if (read_interface(client_config.intf_send, &client_config.ifidx_send, NULL, client_config.arp_send) < 0)
			return 1;
	}

	if(violenceTest)
	{
		client_config.arp[0] = 0x00;
		client_config.arp[1] = 0x11;
		client_config.arp[2] = 0x22;
		client_config.arp[3] = 0x35;
		client_config.arp[4] = 0x58;
	}

VIOLENCE_TEST_START:
	if(violenceTest)
	{
		violenceCount++;
		if(violenceCount > violenceTest) return 1;
		client_config.arp[5] = violenceCount;
	}

	/* if not set, and not suppressed, setup the default client ID */
	if (!client_config.clientid && !no_clientid) {
		client_config.clientid = xmalloc(6 + 3);
		client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
		client_config.clientid[OPT_LEN] = 7;
		client_config.clientid[OPT_DATA] = 1;
		memcpy(client_config.clientid + 3, client_config.arp, 6);
	}

	if (!client_config.vendorclass) {
		client_config.vendorclass = xmalloc(sizeof("udhcp "VERSION) + 2);
		client_config.vendorclass[OPT_CODE] = DHCP_VENDOR;
		client_config.vendorclass[OPT_LEN] = sizeof("udhcp "VERSION) - 1;
		client_config.vendorclass[OPT_DATA] = 1;
		memcpy(&client_config.vendorclass[OPT_DATA], 
			"udhcp "VERSION, sizeof("udhcp "VERSION) - 1);
	}


	/* setup the signal pipe */
	udhcp_sp_setup();
#if 0
	state = INIT_SELECTING;
	run_script(NULL, "deconfig");
	change_mode(LISTEN_RAW);
#else
	perform_release();
	perform_renew();
#endif

	for (;;) {

		tv.tv_sec = timeout - uptime();
		tv.tv_usec = 0;

		if (listen_mode != LISTEN_NONE && fd < 0) {
			if (listen_mode == LISTEN_KERNEL)
				fd = listen_socket(INADDR_ANY, CLIENT_PORT, client_config.interface);
			else
				fd = raw_socket(client_config.ifindex);
			if (fd < 0) {
				LOG(LOG_ERR, "FATAL: couldn't listen on socket, %m");
				return 0;
			}
		}
		max_fd = udhcp_sp_fd_set(&rfds, fd);

		if (tv.tv_sec > 0) {
			DEBUG(LOG_INFO, "Waiting on select...");
			retval = select(max_fd + 1, &rfds, NULL, NULL, &tv);
		} else retval = 0; /* If we already timed out, fall through */

		now = uptime();
		if (retval == 0) {
			/* timeout dropped to zero */
			switch (state) {
			case INIT_SELECTING:
				if(notCheckMii == 0)
				{
#if TARGET //jaykung
					MiiCheckLink(client_config.interface,&link_status);
					//LOG(LOG_INFO, "link_status=>%s\n",(link_status==1)?"Link ok":"No link");
					while(link_status==0)
					{
						//LOG(LOG_INFO, "Wait for %s ready...\n",client_config.interface);
						sleep(3);
						MiiCheckLink(client_config.interface,&link_status);
						//LOG(LOG_INFO, "link_status=>%s\n",(link_status==1)?"Link ok":"No link");
					}
#endif //~jay
				}

				if (packet_num < 3) {
					if (packet_num == 0)
						xid = random_xid();

					/* send discover packet */
					send_discover(xid, requested_ip); /* broadcast */

					timeout = now + ((packet_num == 2) ? 4 : 2);
					packet_num++;
				} else {
					run_script(NULL, "leasefail");
					if (client_config.background_if_no_lease) {
						// LOG(LOG_INFO, "No lease, forking to background.");
						client_background();
					} else if (client_config.abort_if_no_lease) {
						// LOG(LOG_INFO, "No lease, failing.");
						return 1;
				  	}
					/* wait to try again */
					packet_num = 0;
					backoffTime = ((backoffTime<6) ? backoffTime+1 : 6);
					// LOG(LOG_INFO, "backoffTime =%d \n",backoffTime);

					if(backoffForAWhile && backoffTime == 6)
					{
						system("sysconf_cli inform dhcpcStopBackoff");
						timeout = 0x7fffffff;
						break;
					}

					/*It need to have zeroconfig using option "Z" when LAN DHCPC is back off forever (not enable option "A").*/
					if(clientAtLanPort && hasZeroConfig && backoffTime == 2) {
						system("sysconf_cli inform dhcpcStopBackoff");
					}

#if 1 //jaykung 20070303 change timeout
					// LOG(LOG_INFO, "retry =%d \n",retryTime/3);
					{
						int res, cnt;

						res = 1;
						for(cnt = 0; cnt < backoffTime; ++cnt)
						{
							res *= 2;
						}
						timeout = now + res;
					}
					//timeout = now + ( (int)(pow(2,backoffTime)) );
#else
					timeout = now + 60;
#endif
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
				if(notCheckMii == 0)
				{
#if TARGET //jaykung 20070104 check if interface is linked or not
					MiiCheckLink(client_config.interface,&link_status);
					//LOG(LOG_INFO, "No link_status=>%s\n",(link_status==1)?"Link ok":"No link");
					while(link_status==0)
					{
						//LOG(LOG_INFO, "Wait for %s ready...\n",client_config.interface);
						sleep(3);
						MiiCheckLink(client_config.interface,&link_status);
						//LOG(LOG_INFO, "No link_status=>%s\n",(link_status==1)?"Link ok":"No link");
					}
#endif
				}

				if (packet_num < 3) {
					/* send request packet */
					if (state == RENEW_REQUESTED)
						send_renew(xid, server_addr, requested_ip); /* unicast */
					else send_selecting(xid, server_addr, requested_ip); /* broadcast */

					timeout = now + ((packet_num == 2) ? 10 : 2);
					packet_num++;
				} else {
					/* timed out, go back to init state */
					if (state == RENEW_REQUESTED) run_script(NULL, "deconfig");
					state = INIT_SELECTING;
					timeout = now;
					packet_num = 0;
					change_mode(LISTEN_RAW);
				}
				break;
			case BOUND:
				/* Lease is starting to run out, time to enter renewing state */
				state = RENEWING;
				change_mode(LISTEN_KERNEL);
				DEBUG(LOG_INFO, "Entering renew state");
				/* fall right through */
			case RENEWING:
				/* Either set a new T1, or enter REBINDING state */
				if ((t2 - t1) <= (lease / 14400 + 1)) {
					/* timed out, enter rebinding state */
					state = REBINDING;
					timeout = now + (t2 - t1);
					DEBUG(LOG_INFO, "Entering rebinding state");
				} else {
					/* send a request packet */
					send_renew(xid, server_addr, requested_ip); /* unicast */

					t1 = (t2 - t1) / 2 + t1;
					timeout = t1 + start;
				}
				break;
			case REBINDING:
				/* Either set a new T2, or enter INIT state */
				if ((lease - t2) <= (lease / 14400 + 1)) {
					/* timed out, enter init state */
					state = INIT_SELECTING;
					LOG(LOG_INFO, "Lease lost, entering init state");
					run_script(NULL, "deconfig");
					timeout = now;
					packet_num = 0;
					change_mode(LISTEN_RAW);
				} else {
					/* send a request packet */
					send_renew(xid, 0, requested_ip); /* broadcast */

					t2 = (lease - t2) / 2 + t2;
					timeout = t2 + start;
				}
				break;
			case RELEASED:
				/* yah, I know, *you* say it would never happen */
				timeout = 0x7fffffff;
				break;
			}
		} else if (retval > 0 && listen_mode != LISTEN_NONE && FD_ISSET(fd, &rfds)) {
			/* a packet is ready, read it */

			if (listen_mode == LISTEN_KERNEL)
				len = get_packet(&packet, fd);
			else len = get_raw_packet(&packet, fd);

			if (len == -1 && errno != EINTR) {
				DEBUG(LOG_INFO, "error on read, %m, reopening socket");
				change_mode(listen_mode); /* just close and reopen */
			}
			if (len < 0) continue;

			if (packet.xid != xid) {
				DEBUG(LOG_INFO, "Ignoring XID %lx (our xid is %lx)",
					(unsigned long) packet.xid, xid);
				continue;
			}
			/* Ignore packets that aren't for us */
			if (memcmp(client_config.arp,packet.chaddr,6))
			{
				if(client_config.ifidx_send == 0)
				{
					continue;
				}
				// 2013-10-31 Norkay, check sending packet's interface MAC, also
				if(memcmp(client_config.arp_send,packet.chaddr,6))
				{
					continue;
				}
			}

			if ((message = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
				DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
				continue;
			}

			switch (state) {
			case INIT_SELECTING:
				/* Must be a DHCPOFFER to one of our xid's */
				if (*message == DHCPOFFER) {
					if ((temp = get_option(&packet, DHCP_SERVER_ID))) {
						memcpy(&server_addr, temp, 4);
						xid = packet.xid;
						requested_ip = packet.yiaddr;

						/* enter requesting state */
						state = REQUESTING;
						timeout = now;
						packet_num = 0;
					} else {
						DEBUG(LOG_ERR, "No server ID in message");
					}
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
			case RENEWING:
			case REBINDING:
				if (*message == DHCPACK) {
					if (!(temp = get_option(&packet, DHCP_LEASE_TIME))) {
						LOG(LOG_ERR, "No lease time with ACK, using 1 hour lease");
						lease = 60 * 60;
					} else {
						memcpy(&lease, temp, 4);
						lease = ntohl(lease);
					}

					/* enter bound state */
					t1 = lease / 2;

					/* little fixed point for n * .875 */
					t2 = (lease * 0x7) >> 3;
					temp_addr.s_addr = packet.yiaddr;
					LOG(LOG_INFO, "Lease of %s obtained, lease time %ld", inet_ntoa(temp_addr), lease);
#if HAS_SYSTEM_LOG
                    SystemLog("DHCP Client, Lease of %s obtained, lease time %ld", inet_ntoa(temp_addr), lease);
#endif
					start = now;
					timeout = t1 + start;
					requested_ip = packet.yiaddr;

					printf("running normal script\n");

					if(vpn_call != -1)
					{
                        /* only use ifconfig dev to set IP. Not modify /etc/resolv.conf & route  */
						run_script(&packet, "simple");
					}
					else
					{
						run_script(&packet, ((state == RENEWING || state == REBINDING) ? "renew" : "bound"));
					}

					state = BOUND;
					change_mode(LISTEN_NONE);

					if(violenceTest)
					{
						/* if get IP is 192.168.x.x, it will go on the loop.
                           if get IP isn't 192.168.x.x, it will jump the loop. 
						   0xa8c0 = 192.168 */
                        if((packet.yiaddr & 0xffff) == 0xa8c0)
						{
							goto VIOLENCE_TEST_START;
						}
					}

#if HAS_GRAT_ARP /* cfho 2008-0710 */
                    GratArp(packet.yiaddr,client_config.arp,client_config.interface);
                    GratArp(packet.yiaddr,client_config.arp,client_config.interface);
                    GratArp(packet.yiaddr,client_config.arp,client_config.interface);
#endif
#if HAS_NETBIOS_FUNCTION
					if (!(fp_NetBios = fopen(DHCP_NETBIOS_LEARN_FROM_WAN_FILE, "w")))
					{
						printf("failed to write %s\n", DHCP_NETBIOS_LEARN_FROM_WAN_FILE);
						return -1;
					}

					/* get the setting of WINS IP address */
					message = get_option(&packet, DHCP_WINS_SERVER);

					if(message)
					{
						isLearnFromWanEnable = 1;
						option_wins_ip_len = *(message-1);

						switch(option_wins_ip_len)
						{
							case NETBIOS_WINS_IP_1_AND_2_LEN:
								memcpy(&wins_ip_2, message+4, 4);
								wins_ip_2 = ntohl(wins_ip_2);
								fprintf(fp_NetBios, "wins_ip_2 %ld\n", wins_ip_2);
							/* continue to set the primary WINS IP Address */
							case NETBIOS_WINS_IP_1_LEN:
								memcpy(&wins_ip_1, message, 4);
								wins_ip_1 = ntohl(wins_ip_1);
								fprintf(fp_NetBios, "wins_ip_1 %ld\n", wins_ip_1);
							break;
							default:
								printf("Unrecognized WINS IP address\n");
							break;
						}
					}

					/* get the setting of node type */
					message = get_option(&packet, DHCP_NODE_TYPE);

					if(message)
					{
						isLearnFromWanEnable = 1;
						nodeType = *message;
						fprintf(fp_NetBios, "node_type %d\n", nodeType);
					}

					/* get the setting of scopt */
					message = get_option(&packet, DHCP_SCOPE);

					if(message != NULL)
					{
						isLearnFromWanEnable = 1;
						scope_len = *(message-1) + 1; // +1 is for the character '\0'
						snprintf(scope, scope_len, "%s", message);
						fprintf(fp_NetBios, "scope %s\n", scope);
					}

					fclose(fp_NetBios);

					/* inform node type */
					sprintf(commands, "sysconf_cli inform netbios_from_wan %d", isLearnFromWanEnable);
					system(commands);
#endif

					// Nofity sysconfig to monitor ip status
                    if(vpn_call != -1 || wan_detect == 0 || clientAtLanPort == 1 || clientAtWLanPort)
					{
						FILE *f;

						if (!(f = fopen(DHCP_LOG_FILE, "w")))
						{
						    printf("failed to write %s\n", DHCP_LOG_FILE);
						    return -1;
						}
						fprintf(f,"%ld", lease);
						fclose(f);

						LOG(LOG_INFO, "send \"dhcp ip obtained\" to sysconfd\n");

						// Order is important
						if(vpn_call != -1)
						{
							sprintf(commands, "sysconf_cli ip_inform vpn-call%d", vpn_call);
							system(commands);
						}
						else if(clientAtLanPort == 1)
						{
							if(hasZeroConfig) {
								backoffTime = 0; //clear backofftime
							}
							system("sysconf_cli ip_inform lan-ip-Obtained");
						}
                        else if(clientAtWLanPort)
                        {
							system("sysconf_cli ip_inform wlan-ip-Obtained");
                        }
						else if(wan_detect == 0)
						{
						    //unsigned int rip;
						    							
							message = get_option(&packet, DHCP_ROUTER);

                            /* get router ip addr */
							if(message)
							{
								rip_addr.s_addr = *(unsigned int*)message;
								printf("Router = %s\n", inet_ntoa(rip_addr));
							}
							
						    /* inform our IP address */
						    sprintf(commands, "sysconf_cli ip_inform ip-Obtained %d %s", key_index, inet_ntoa(temp_addr));
							system(commands);
						}
					}
					else
					{
                        return 0;
					}

					if (client_config.quit_after_lease)
						return 0;
					if (!client_config.foreground)
						client_background();


				} else if (*message == DHCPNAK) {
					/* return to init state */
					LOG(LOG_INFO, "Received DHCP NAK");
					run_script(&packet, "nak");
					if (state != REQUESTING)
						run_script(NULL, "deconfig");
					state = INIT_SELECTING;
					timeout = now;
					requested_ip = 0;
					packet_num = 0;
					change_mode(LISTEN_RAW);
					sleep(3); /* avoid excessive network traffic */
				}
				break;
			/* case BOUND, RELEASED: - ignore all packets */
			}
		} else if (retval > 0 && (sig = udhcp_sp_read(&rfds))) {
			switch (sig) {
			case SIGUSR1:
				backoffTime=0;//clear backofftime jay
				perform_renew();
				break;
			case SIGUSR2:
				backoffTime=0; //clear backofftime jay
				perform_release();
				break;
			case SIGTERM:
				LOG(LOG_INFO, "Received SIGTERM");
				return 0;
			}
		} else if (retval == -1 && errno == EINTR) {
			/* a signal was caught */
		} else {
			/* An error occured */
			DEBUG(LOG_ERR, "Error on select");
		}

	}
	return 0;
}
