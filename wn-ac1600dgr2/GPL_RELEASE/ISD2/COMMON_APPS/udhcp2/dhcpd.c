/* vi: set sw=4 ts=4: */
/* dhcpd.c
 *
 * udhcp Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include <syslog.h>
#include "common.h"
#include "dhcpd.h"
#include "options.h"
#ifdef UDHCP_WITHOUT_BUSYBOX
#include "pidfile.h"
#endif


/* globals */
struct dhcpOfferedAddr *leases;
struct server_config_t server_config;
static char hostname_align[HOSTNAME_LEN+1];


#ifndef COMBINED_BINARY

#ifndef UDHCP_WITHOUT_BUSYBOX
#ifdef __GLIBC__
/* Make it reside in R/W memory: */
int *const bb_errno __attribute__ ((section (".data")));
#endif
#endif

const char *applet_name = "udhcpd";
#define udhcpd_trivial_usage \
       "[-fS] [configfile]" \

#define udhcpd_full_usage \
       "DHCP server" \
       "\n	-f	Stay in foreground" \
       "\n	-S	Log to syslog too"

void bb_show_usage(void)
{
	fprintf(stderr, "Usage: udhcpd %s\n", udhcpd_trivial_usage);
	fprintf(stderr, udhcpd_full_usage);
	fprintf(stderr, "\n");
	exit(1);
}
#endif

#ifdef COMBINED_BINARY
int udhcpd_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	fd_set rfds;
	struct timeval tv;
	int server_socket = -1, bytes, retval, max_sock;
	struct dhcpMessage packet;
	uint8_t *state, *server_id, *requested, *hostname;
	uint32_t server_id_align, requested_align, static_lease_ip;
	unsigned timeout_end;
	unsigned num_ips;
	unsigned opt;
	struct option_set *option;
	struct dhcpOfferedAddr *lease, static_lease;
	// uint32_t scan_ip;
	uint32_t r;
	unsigned int hostname_len;
#if HAS_ARP_PROXY
	char cli_tmp[100];
	struct in_addr offeradd;
#endif


#ifdef UDHCP_WITHOUT_BUSYBOX
	read_config(argc < 2 ? DHCPD_CONF_FILE : argv[1]);
#else

#ifdef __GLIBC__
	(*(int **)&bb_errno) = __errno_location();
#endif

	opt = getopt32(argv, "fS");
	argv += optind;

	if (!(opt & 1)) { /* no -f */
		bb_daemonize_or_rexec(0, argv);
		logmode &= ~LOGMODE_STDIO;
	}

	if (opt & 2) { /* -S */
		openlog(applet_name, LOG_PID, LOG_LOCAL0);
		logmode |= LOGMODE_SYSLOG;
	}

	/* Would rather not do read_config before daemonization -
	 * otherwise NOMMU machines will parse config twice */
	read_config(argv[0] ? argv[0] : DHCPD_CONF_FILE);

	/* Make sure fd 0,1,2 are open */
	bb_sanitize_stdio();
#endif
	/* Equivalent of doing a fflush after every \n */
	setlinebuf(stdout);

	/* Create pidfile */
#ifdef UDHCP_WITHOUT_BUSYBOX
	{
		int pid_fd;
		pid_fd = pidfile_acquire(server_config.pidfile);
		pidfile_write_release(pid_fd);
	}
#else
	write_pidfile(server_config.pidfile);
#endif
	/* if (!..) bb_perror_msg("cannot create pidfile %s", pidfile); */

	bb_info_msg("%s (v%s) started", applet_name, BB_VER);
       
	option = find_option(server_config.options, DHCP_LEASE_TIME);
	server_config.lease = LEASE_TIME;
	if (option) {
		memcpy(&server_config.lease, option->data + 2, 4);
		server_config.lease = ntohl(server_config.lease);
	}

	/* Sanity check */
	num_ips = server_config.end_ip - server_config.start_ip + 1;
	if (server_config.max_leases > num_ips) {
		bb_error_msg("max_leases=%u is too big, setting to %u",
			(unsigned)server_config.max_leases, num_ips);
		server_config.max_leases = num_ips;
	}

	/* HW limitation, only support at most class B clients */
	/* If do not limit it, DHCPD will crash when set class A clients */
	
	if(server_config.max_leases > MAX_CLIENTS)
		server_config.max_leases = MAX_CLIENTS;

#ifdef UDHCP_WITHOUT_BUSYBOX
	leases = xcalloc(server_config.max_leases, sizeof(struct dhcpOfferedAddr));
#else
	leases = xzalloc(server_config.max_leases * sizeof(*leases));
#endif
	read_leases(server_config.lease_file);

	if (read_interface(server_config.interface, &server_config.ifindex,
			   &server_config.server, server_config.arp)) {
		retval = 1;
		goto ret;
	}
/*        
        for(scan_ip=server_config.start_ip; scan_ip<server_config.end_ip; scan_ip++)
        {
            arpping(htonl(scan_ip), 
                    server_config.server, 
                    server_config.arp, 
                    server_config.interface);
        }
*/
	/* Setup the signal pipe */
	udhcp_sp_setup();

	timeout_end = monotonic_sec() + server_config.auto_time;
	while (1) { /* loop until universe collapses */

		if (server_socket < 0) {
			server_socket = listen_socket(/*INADDR_ANY,*/ SERVER_PORT,
					server_config.interface);
		}

		max_sock = udhcp_sp_fd_set(&rfds, server_socket);
		if (server_config.auto_time) {
			tv.tv_sec = timeout_end - monotonic_sec();
			tv.tv_usec = 0;
		}
		retval = 0;
		if (!server_config.auto_time || tv.tv_sec > 0) {
			retval = select(max_sock + 1, &rfds, NULL, NULL,
					server_config.auto_time ? &tv : NULL);
		}
		if (retval == 0) {
			write_leases();
			timeout_end = monotonic_sec() + server_config.auto_time;
			continue;
		}
		if (retval < 0 && errno != EINTR) {
			DEBUG("error on select");
			continue;
		}

		switch (udhcp_sp_read(&rfds)) {
		case SIGUSR1:
			bb_info_msg("Received a SIGUSR1");
			write_leases();
			/* why not just reset the timeout, eh */
			timeout_end = monotonic_sec() + server_config.auto_time;
			continue;
		case SIGUSR2:
			bb_info_msg("Received a SIGUSR2");
			memset(leases, 0, server_config.max_leases * sizeof(struct dhcpOfferedAddr));
			/* reload lease file */
			read_leases(server_config.lease_file);
			write_leases();
			continue;
		case SIGTERM:
			bb_info_msg("Received a SIGTERM");
			goto ret0;
		case 0: break;		/* no signal */
		default: continue;	/* signal or error (probably EINTR) */
		}

		bytes = udhcp_get_packet(&packet, server_socket); /* this waits for a packet - idle */
		if (bytes < 0) {
			if (bytes == -1 && errno != EINTR) {
				DEBUG("error on read, %s, reopening socket", strerror(errno));
				close(server_socket);
				server_socket = -1;
			}
			continue;
		}

		state = get_option(&packet, DHCP_MESSAGE_TYPE);
		if (state == NULL) {
			bb_error_msg("cannot get option from packet, ignoring");
			continue;
		}

		/* Look for a static lease */
		static_lease_ip = getIpByMac(server_config.static_leases, &packet.chaddr);

		if (static_lease_ip) {
			bb_info_msg("Found static lease: %x", static_lease_ip);

			memcpy(&static_lease.chaddr, &packet.chaddr, 16);
			static_lease.yiaddr = static_lease_ip;
			static_lease.expires = 0;

			lease = &static_lease;
		} else {
			lease = find_lease_by_chaddr(packet.chaddr);
		}

		switch (state[0]) {
		case DHCPDISCOVER:
			DEBUG("Received DISCOVER");

			if (sendOffer(&packet) < 0) {
				bb_error_msg("send OFFER failed");
			}
			break;
		case DHCPREQUEST:
			DEBUG("received REQUEST");

			requested = get_option(&packet, DHCP_REQUESTED_IP);
			server_id = get_option(&packet, DHCP_SERVER_ID);
			hostname  = get_option(&packet, DHCP_HOST_NAME);
			hostname_align[0]=0;

			if (hostname)
			{
				hostname_len = *(hostname-1);
				if(hostname_len>HOSTNAME_LEN) hostname_len=HOSTNAME_LEN;
				memcpy(&hostname_align,  hostname,  hostname_len);
				hostname_align[hostname_len]=0;
			}
			
			if (requested) memcpy(&requested_align, requested, 4);
			if (server_id) memcpy(&server_id_align, server_id, 4);

			if (lease) {
#if HAS_ARP_PROXY
				offeradd.s_addr = lease->yiaddr;
				sprintf(cli_tmp, "arpproxy_cli modify ip_leasetime %s %d", inet_ntoa(offeradd),server_config.lease);
				system(cli_tmp);
#endif
				if (server_id) {
					/* SELECTING State */
					DEBUG("server_id = %08x", ntohl(server_id_align));
					if (server_id_align == server_config.server && requested && requested_align == lease->yiaddr) {
						sendACK(&packet, lease->yiaddr, hostname_align);
					}
				} else if (requested) {
					/* INIT-REBOOT State */
					if (lease->yiaddr == requested_align) {
						if (reservedIp(server_config.static_leases, lease->yiaddr) && static_lease_ip != lease->yiaddr) {
							/* the IP is reserved. */
							/* The MAC is NOT for the reserved IP  */
							sendNAK(&packet);
						} else {
							sendACK(&packet, lease->yiaddr, hostname_align);
						}
					} else {
						sendNAK(&packet);
					}
				} else if (lease->yiaddr == packet.ciaddr) {
					/* RENEWING or REBINDING State */
					if (reservedIp(server_config.static_leases, lease->yiaddr) && static_lease_ip != lease->yiaddr) {
						/* the IP is reserved. */
						/* The MAC is NOT for the reserved IP  */
						sendNAK(&packet);
					} else {
						sendACK(&packet, lease->yiaddr, hostname_align);
					}
				} else {
					/* don't know what to do!!!! */
					sendNAK(&packet);
				}

			/* what to do if we have no record of the client */
			} else if (server_id) {
				/* SELECTING State */

			} else if (requested) {
				/* INIT-REBOOT State */
				lease = find_lease_by_yiaddr(requested_align);
				if (lease) {
					if (lease_expired(lease)) {
						/* probably best if we drop this lease */
						memset(lease->chaddr, 0, 16);
					/* make some contention for this address */
					} else
						sendNAK(&packet);
				} else {
					r = ntohl(requested_align);
					if (r < server_config.start_ip || r > server_config.end_ip) {
						sendNAK(&packet);
					} else if (reservedIp(server_config.static_leases, requested_align) && static_lease_ip != requested_align) {
						/* the IP is reserved. */
						/* The MAC is NOT for the reserved IP  */
						sendNAK(&packet);
					} else {
						sendACK(&packet, requested_align, hostname_align);
					}
					/* else remain silent */
				}

			} else {
				/* RENEWING or REBINDING State */
					r = ntohl(packet.ciaddr);
					if (r < server_config.start_ip || r > server_config.end_ip ) {
						sendNAK(&packet);
					} else if (reservedIp(server_config.static_leases, packet.ciaddr) && static_lease_ip != packet.ciaddr) {
						/* the IP is reserved. */
						/* The MAC is NOT for the reserved IP  */
						sendNAK(&packet);
					} else {
						sendACK(&packet, packet.ciaddr, hostname_align);
					}
			}
			break;
		case DHCPDECLINE:
			DEBUG("Received DECLINE");
			if (lease) {
				memset(lease->chaddr, 0, 16);
#if 1 //cfho 2008-0109
                lease->expires = monotonic_sec() + server_config.decline_time;
#else
				lease->expires = time(0) + server_config.decline_time;
#endif
			}
			break;
		case DHCPRELEASE:
			DEBUG("Received RELEASE");
#if 1 //cfho 2008-0109
            if (lease)
                lease->expires = monotonic_sec();
#else
			if (lease)
				lease->expires = time(0);
#endif
			break;
		case DHCPINFORM:
			DEBUG("Received INFORM");
			send_inform(&packet);
			break;
		default:
			bb_info_msg("Unsupported DHCP message (%02x) - ignoring", state[0]);
		}
	}
 ret0:
	retval = 0;
 ret:
	/*if (server_config.pidfile) - server_config.pidfile is never NULL */
#ifndef UDHCP_WITHOUT_BUSYBOX
		remove_pidfile(server_config.pidfile);
#endif
	return retval;
}
