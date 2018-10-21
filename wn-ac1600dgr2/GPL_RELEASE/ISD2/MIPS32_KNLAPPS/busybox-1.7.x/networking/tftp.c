/* vi: set sw=4 ts=4: */
/* -------------------------------------------------------------------------
 * tftp.c
 *
 * A simple tftp client for busybox.
 * Tries to follow RFC1350.
 * Only "octet" mode supported.
 * Optional blocksize negotiation (RFC2347 + RFC2348)
 *
 * Copyright (C) 2001 Magnus Damm <damm@opensource.se>
 *
 * Parts of the code based on:
 *
 * atftp:  Copyright (C) 2000 Jean-Pierre Lefebvre <helix@step.polymtl.ca>
 *                        and Remi Lefebvre <remi@debian.org>
 *
 * utftp:  Copyright (C) 1999 Uwe Ohse <uwe@ohse.de>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 * ------------------------------------------------------------------------- */

#include "libbb.h"


#if ENABLE_FEATURE_TFTP_GET || ENABLE_FEATURE_TFTP_PUT

#define TFTP_BLOCKSIZE_DEFAULT 512	/* according to RFC 1350, don't change */
#define TFTP_TIMEOUT 5	/* seconds */
#define TFTP_NUM_RETRIES 5 /* number of retries */

/* opcodes we support */
#define TFTP_RRQ   1
#define TFTP_WRQ   2
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TFTP_ERROR 5
#define TFTP_OACK  6

#if ENABLE_FEATURE_TFTP_GET && !ENABLE_FEATURE_TFTP_PUT
#define USE_GETPUT(...)
#define CMD_GET(cmd) 1
#define CMD_PUT(cmd) 0
#elif !ENABLE_FEATURE_TFTP_GET && ENABLE_FEATURE_TFTP_PUT
#define USE_GETPUT(...)
#define CMD_GET(cmd) 0
#define CMD_PUT(cmd) 1
#else
#define USE_GETPUT(...) __VA_ARGS__
/* masks coming from getpot32 */
#define CMD_GET(cmd) ((cmd) & 1)
#define CMD_PUT(cmd) ((cmd) & 2)
#endif
/* NB: in the code below
 * CMD_GET(cmd) and CMD_PUT(cmd) are mutually exclusive
 */

#include <linux/types.h>
#if (defined(__GLIBC__) && __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1) || defined _NEWLIB_VERSION
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif
#include <net/if.h>
#include <netinet/udp.h>
#include <netinet/ip.h>

struct udp_dhcp_packet {
	struct iphdr ip;
	struct udphdr udp;
	char data[TFTP_BLOCKSIZE_DEFAULT];
};

static int gifindex = 0;
static int gPort = 0;
static uint32_t gAddr = 0;
static uint8_t gDst_mac[6];
const uint8_t *gDest_arp = NULL;
static int gTimeout = TFTP_TIMEOUT;

static int xx_read_interface(const char *interface, int *ifindex, uint32_t *addr, uint8_t *arp)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *our_ip;

	memset(&ifr, 0, sizeof(ifr));
	fd = xsocket(AF_INET, SOCK_RAW, IPPROTO_RAW);

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
	if (addr) {
		if (ioctl_or_perror(fd, SIOCGIFADDR, &ifr,
			"is interface %s up and configured?\n", interface)
		) {
			close(fd);
			return -1;
		}
		our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
		*addr = our_ip->sin_addr.s_addr;
#if ENABLE_DEBUG_TFTP
		fprintf(stderr, "%s (our ip) = %s\n", ifr.ifr_name, inet_ntoa(our_ip->sin_addr));
#endif
	}

	if (ifindex) {
		if (ioctl_or_warn(fd, SIOCGIFINDEX, &ifr) != 0) {
			close(fd);
			return -1;
		}
#if ENABLE_DEBUG_TFTP
		fprintf(stderr, "adapter index %d\n", ifr.ifr_ifindex);
#endif
		*ifindex = ifr.ifr_ifindex;
	}

	if (arp) {
		if (ioctl_or_warn(fd, SIOCGIFHWADDR, &ifr) != 0) {
			close(fd);
			return -1;
		}
		memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
#if ENABLE_DEBUG_TFTP
		fprintf(stderr, "adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x\n",
			arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
#endif
	}

	close(fd);
	return 0;
}

static uint16_t xx_checksum(void *addr, int count)
{
	/* Compute Internet Checksum for "count" bytes
	 *         beginning at location "addr".
	 */
	int32_t sum = 0;
	uint16_t *source = (uint16_t *) addr;

	while (count > 1)  {
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0) {
		/* Make sure that the left-over byte is added correctly both
		 * with little and big endian hosts */
		uint16_t tmp = 0;
		*(uint8_t *) (&tmp) = * (uint8_t *) source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

static int xx_raw_packet(char *payload, int send_len, uint32_t source_ip, int source_port,
						 uint32_t dest_ip, int dest_port, const uint8_t *dest_arp, int ifindex)
{
	static int fd = -1;
	int result;
	static struct sockaddr_ll dest;
	struct udp_dhcp_packet packet;

	/* accelerate, not do socket and bind every time */
	if(fd == -1)
	{
		fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
		if (fd < 0) {
			bb_perror_msg("socket");
			return -1;
		}
		memset(&dest, 0, sizeof(dest));
		dest.sll_family = AF_PACKET;
		dest.sll_protocol = htons(ETH_P_IP);
		dest.sll_ifindex = ifindex;
		dest.sll_halen = 6;
		memcpy(dest.sll_addr, dest_arp, 6);
		if (bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0) {
			bb_perror_msg("bind");
			close(fd);
			return -1;
		}
	}
	memset(&packet, 0, sizeof(packet));

	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = source_ip;
	packet.ip.daddr = dest_ip;
	packet.udp.source = htons(source_port);
	packet.udp.dest = htons(dest_port);
	packet.udp.len = htons(sizeof(packet.udp) + send_len); /* cheat on the psuedo-header */
	packet.ip.tot_len = packet.udp.len;
	memcpy(&(packet.data), payload, send_len);
	packet.udp.check = xx_checksum(&packet, sizeof(struct iphdr)+sizeof(struct udphdr)+send_len);

	packet.ip.tot_len = htons(sizeof(struct iphdr)+sizeof(struct udphdr)+send_len);
	packet.ip.ihl = sizeof(packet.ip) >> 2;
	packet.ip.version = IPVERSION;
	packet.ip.ttl = IPDEFTTL;
	packet.ip.check = xx_checksum(&(packet.ip), sizeof(packet.ip));

	result = sendto(fd, &packet, sizeof(struct iphdr)+sizeof(struct udphdr)+send_len, 0,
			(struct sockaddr *) &dest, sizeof(dest));
	if (result <= 0) {
		bb_perror_msg("sendto");
	}
	//close(fd);
	return result;
}

static void xx_sendto(char* xbuf, int send_len, int port, const uint8_t *dest_arp)
{
	// fprintf(stderr, "sendto port--->%d\n", ntohs(port));
	xx_raw_packet(xbuf, send_len,
				  INADDR_ANY, gPort+100, 
				  INADDR_BROADCAST, ntohs(port),
				  dest_arp ? dest_arp : (uint8_t *) "\xff\xff\xff\xff\xff\xff", 
				  gifindex);
}

#if ENABLE_FEATURE_TFTP_BLOCKSIZE

static int tftp_blocksize_check(int blocksize, int bufsize)
{
	/* Check if the blocksize is valid:
	 * RFC2348 says between 8 and 65464,
	 * but our implementation makes it impossible
	 * to use blocksizes smaller than 22 octets.
	 */

	if ((bufsize && (blocksize > bufsize))
	 || (blocksize < 8) || (blocksize > 65564)
	) {
		bb_error_msg("bad blocksize");
		return 0;
	}

	return blocksize;
}

static char *tftp_option_get(char *buf, int len, const char *option)
{
	int opt_val = 0;
	int opt_found = 0;
	int k;

	while (len > 0) {
		/* Make sure the options are terminated correctly */
		for (k = 0; k < len; k++) {
			if (buf[k] == '\0') {
				goto nul_found;
			}
		}
		return NULL;
 nul_found:
		if (opt_val == 0) {
			if (strcasecmp(buf, option) == 0) {
				opt_found = 1;
			}
		} else if (opt_found) {
			return buf;
		}

		k++;
		buf += k;
		len -= k;
		opt_val ^= 1;
	}

	return NULL;
}

#endif

static int tftp( USE_GETPUT(const int cmd,)
		len_and_sockaddr *peer_lsa,
		const char *remotefile, const int localfd,
		unsigned port, int tftp_bufsize)
{
	struct timeval tv;
	fd_set rfds;
	int socketfd;
	int len;
	int send_len;
	USE_FEATURE_TFTP_BLOCKSIZE(smallint want_option_ack = 0;)
	smallint finished = 0;
	uint16_t opcode;
	uint16_t block_nr = 1;
	uint16_t recv_blk;
	int timeout = TFTP_NUM_RETRIES;
	char *cp;

	unsigned org_port;
	len_and_sockaddr *const from = alloca(offsetof(len_and_sockaddr, sa) + peer_lsa->len);

	/* Can't use RESERVE_CONFIG_BUFFER here since the allocation
	 * size varies meaning BUFFERS_GO_ON_STACK would fail */
	/* We must keep the transmit and receive buffers seperate */
	/* In case we rcv a garbage pkt and we need to rexmit the last pkt */
	char *xbuf = xmalloc(tftp_bufsize += 4);
	char *rbuf = xmalloc(tftp_bufsize);

	port = org_port = htons(gPort);

	if(gifindex)
	{
		struct sockaddr_in addr;

		socketfd = xsocket (PF_INET, SOCK_DGRAM, 0);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(gPort+100);
		addr.sin_addr.s_addr = INADDR_ANY;
	
		if(bind(socketfd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
		{
			bb_perror_msg("bind(PF_UNIX)");
			return -1;
		}
	}
	else
	{
		socketfd = xsocket(peer_lsa->sa.sa_family, SOCK_DGRAM, 0);
	}
	/* build opcode */
	opcode = TFTP_WRQ;
	if (CMD_GET(cmd)) {
		opcode = TFTP_RRQ;
	}
	cp = xbuf + 2;
	/* add filename and mode */
	/* fill in packet if the filename fits into xbuf */
	len = strlen(remotefile) + 1;
	if (2 + len + sizeof("octet") >= tftp_bufsize) {
		bb_error_msg("remote filename is too long");
		goto ret;
	}
	strcpy(cp, remotefile);
	cp += len;
	/* add "mode" part of the package */
	strcpy(cp, "octet");
	cp += sizeof("octet");

#if ENABLE_FEATURE_TFTP_BLOCKSIZE
	len = tftp_bufsize - 4;	/* data block size */
	if (len != TFTP_BLOCKSIZE_DEFAULT) {
		/* rfc2348 says that 65464 is a max allowed value */
		if ((&xbuf[tftp_bufsize - 1] - cp) < sizeof("blksize NNNNN")) {
			bb_error_msg("remote filename is too long");
			goto ret;
		}
		/* add "blksize", <nul>, blocksize */
		strcpy(cp, "blksize");
		cp += sizeof("blksize");
		cp += snprintf(cp, 6, "%d", len) + 1;
		want_option_ack = 1;
	}
#endif
	/* First packet is built, so skip packet generation */
	goto send_pkt;

	/* Using mostly goto's - continue/break will be less clear
	 * in where we actually jump to */

	while (1) {
		/* Build ACK or DATA */
		cp = xbuf + 2;
		*((uint16_t*)cp) = htons(block_nr);
		cp += 2;
		block_nr++;
		opcode = TFTP_ACK;
		if (CMD_PUT(cmd)) {
			opcode = TFTP_DATA;
			len = full_read(localfd, cp, tftp_bufsize - 4);
			if (len < 0) {
				bb_perror_msg(bb_msg_read_error);
				goto ret;
			}
			if (len != (tftp_bufsize - 4)) {
				finished = 1;
			}
			cp += len;
		}
 send_pkt:
		/* Send packet */
		*((uint16_t*)xbuf) = htons(opcode); /* fill in opcode part */
		send_len = cp - xbuf;
		/* NB: send_len value is preserved in code below
		 * for potential resend */
 send_again:
#if ENABLE_DEBUG_TFTP
		fprintf(stderr, "sending %u bytes\n", send_len);
		for (cp = xbuf; cp < &xbuf[send_len]; cp++)
			fprintf(stderr, "%02x ", (unsigned char) *cp);
		fprintf(stderr, "\n");
#endif
		if(gifindex)
		{
			xx_sendto(xbuf, send_len, port, gDest_arp);
		}
		else
		{
			xsendto(socketfd, xbuf, send_len, &peer_lsa->sa, peer_lsa->len);
		}
		/* Was it final ACK? then exit */
		if (finished && (opcode == TFTP_ACK))
			goto ret;

		// This line will cause run forever when server doesn't response.
		// timeout = TFTP_NUM_RETRIES;	/* re-initialize */
 recv_again:
		/* Receive packet */
		tv.tv_sec = gTimeout;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(socketfd, &rfds);
		switch (select(socketfd + 1, &rfds, NULL, NULL, &tv)) {
			unsigned from_port;
		case 1:
			from->len = peer_lsa->len;
			memset(&from->sa, 0, peer_lsa->len);
			len = recvfrom(socketfd, rbuf, tftp_bufsize, 0,
						&from->sa, &from->len);
			if (len < 0) {
				bb_perror_msg("recvfrom");
				goto ret;
			}
			from_port = get_nport(&from->sa);
			if (port == org_port) {
				/* Our first query went to port 69
				 * but reply will come from different one.
				 * Remember and use this new port */
				port = from_port;
				set_nport(peer_lsa, from_port);
			}
			if (port != from_port)
				goto recv_again;
			goto process_pkt;
		case 0:
			timeout--;
			if (timeout == 0) {
				bb_error_msg("last timeout");
				goto ret;
			}
			bb_error_msg("last timeout" + 5);
			goto send_again; /* resend last sent pkt */
		default:
			bb_perror_msg("select");
			goto ret;
		}
 process_pkt:
		/* Process recv'ed packet */
		opcode = ntohs( ((uint16_t*)rbuf)[0] );
		recv_blk = ntohs( ((uint16_t*)rbuf)[1] );

#if ENABLE_DEBUG_TFTP
		fprintf(stderr, "received %d bytes: %04x %04x\n", len, opcode, recv_blk);
#endif

		if (opcode == TFTP_ERROR) {
			static const char *const errcode_str[] = {
				"",
				"file not found",
				"access violation",
				"disk full",
				"illegal TFTP operation",
				"unknown transfer id",
				"file already exists",
				"no such user",
				"bad option"
			};

			const char *msg = "";

			if (rbuf[4] != '\0') {
				msg = &rbuf[4];
				rbuf[tftp_bufsize - 1] = '\0';
			} else if (recv_blk < ARRAY_SIZE(errcode_str)) {
				msg = errcode_str[recv_blk];
			}
			bb_error_msg("server error: (%u) %s", recv_blk, msg);
			goto ret;
		}

#if ENABLE_FEATURE_TFTP_BLOCKSIZE
		if (want_option_ack) {
			want_option_ack = 0;

			if (opcode == TFTP_OACK) {
				/* server seems to support options */
				char *res;

				res = tftp_option_get(&rbuf[2], len - 2, "blksize");
				if (res) {
					int blksize = xatoi_u(res);
					if (!tftp_blocksize_check(blksize, tftp_bufsize - 4)) {
						/* send ERROR 8 to server... */
						/* htons can be impossible to use in const initializer: */
						/*static const uint16_t error_8[2] = { htons(TFTP_ERROR), htons(8) };*/
						/* thus we open-code big-endian layout */
						static const uint8_t error_8[4] = { 0,TFTP_ERROR, 0,8 };
						if(gifindex)
						{
							xx_sendto(error_8, 4, port, gDest_arp);
						}
						else
						{
							xsendto(socketfd, error_8, 4, &peer_lsa->sa, peer_lsa->len);
						}
						bb_error_msg("server proposes bad blksize %d, exiting", blksize);
						goto ret;
					}
#if ENABLE_DEBUG_TFTP
					fprintf(stderr, "using blksize %u\n",
							blksize);
#endif
					tftp_bufsize = blksize + 4;
					/* Send ACK for OACK ("block" no: 0) */
					block_nr = 0;
					continue;
				}
				/* rfc2347:
				 * "An option not acknowledged by the server
				 *  must be ignored by the client and server
				 *  as if it were never requested." */
			}

			bb_error_msg("blksize is not supported by server"
						" - reverting to 512");
			tftp_bufsize = TFTP_BLOCKSIZE_DEFAULT + 4;
		}
#endif
		/* block_nr is already advanced to next block# we expect
		 * to get / block# we are about to send next time */

		if (CMD_GET(cmd) && (opcode == TFTP_DATA)) {
			if (recv_blk == block_nr) {
				len = full_write(localfd, &rbuf[4], len - 4);
				if (len < 0) {
					bb_perror_msg(bb_msg_write_error);
					goto ret;
				}
				if (len != (tftp_bufsize - 4)) {
					finished = 1;
				}
				continue; /* send ACK */
			}
			if (recv_blk == (block_nr - 1)) {
				/* Server lost our TFTP_ACK.  Resend it */
				block_nr = recv_blk;
				continue;
			}
		}

		if (CMD_PUT(cmd) && (opcode == TFTP_ACK)) {
			/* did server ACK our last DATA pkt? */
			if (recv_blk == (uint16_t) (block_nr - 1)) {
				if (finished)
					goto ret;
				continue; /* send next block */
			}
		}
		/* Awww... recv'd packet is not recognized! */
		goto recv_again;
		/* why recv_again? - rfc1123 says:
		 * "The sender (i.e., the side originating the DATA packets)
		 *  must never resend the current DATA packet on receipt
		 *  of a duplicate ACK".
		 * DATA pkts are resent ONLY on timeout.
		 * Thus "goto send_again" will ba a bad mistake above.
		 * See:
		 * http://en.wikipedia.org/wiki/Sorcerer's_Apprentice_Syndrome
		 */
	}
 ret:
	if (ENABLE_FEATURE_CLEAN_UP) {
		close(socketfd);
		free(xbuf);
		free(rbuf);
	}
	return finished == 0; /* returns 1 on failure */
}

int tftp_main(int argc, char **argv);
int tftp_main(int argc, char **argv)
{
	len_and_sockaddr *peer_lsa;
	const char *localfile = NULL;
	const char *remotefile = NULL;
#if ENABLE_FEATURE_TFTP_BLOCKSIZE
	const char *sblocksize = NULL;
#endif
	int port;
	USE_GETPUT(int cmd;)
	int fd = -1;
	int flags = 0;
	int result;
	int blocksize = TFTP_BLOCKSIZE_DEFAULT;
	const char *intf = NULL;
	const char *dst_mac_str = NULL;
	int mac_cnt;
	const char *stimeout = NULL;
	const char *check = NULL;
	int check_cmd = 0;

	/* -p or -g is mandatory, and they are mutually exclusive */
	opt_complementary = "" USE_FEATURE_TFTP_GET("g:") USE_FEATURE_TFTP_PUT("p:")
			USE_GETPUT("?g--p:p--g");

	USE_GETPUT(cmd =) getopt32(argv,
			USE_FEATURE_TFTP_GET("g") USE_FEATURE_TFTP_PUT("p")
				"l:r:i:m:t:c:" USE_FEATURE_TFTP_BLOCKSIZE("b:"),
			&localfile, &remotefile, &intf, &dst_mac_str, &stimeout, &check
			USE_FEATURE_TFTP_BLOCKSIZE(, &sblocksize));
	argv += optind;

	flags = O_RDONLY;
	if (CMD_GET(cmd))
		flags = O_WRONLY | O_CREAT | O_TRUNC;

#if ENABLE_FEATURE_TFTP_BLOCKSIZE
	if (sblocksize) {
		blocksize = xatoi_u(sblocksize);
		if (!tftp_blocksize_check(blocksize, 0)) {
			return EXIT_FAILURE;
		}
	}
#endif

	if(stimeout)
	{
		gTimeout = xatoi_u(stimeout);
	}
	else
	{
		gTimeout = TFTP_TIMEOUT;
	}

	if(check)
	{
		check_cmd = xatoi_u(check);
	}

	if (!localfile)
		localfile = remotefile;
	if (!remotefile)
		remotefile = localfile;
	/* Error if filename or host is not known */
	if (!remotefile || !argv[0])
		bb_show_usage();

	fd = CMD_GET(cmd) ? STDOUT_FILENO : STDIN_FILENO;
	if (!LONE_DASH(localfile)) {
		fd = xopen(localfile, flags);
	}

	port = bb_lookup_port(argv[1], "udp", 69);
	peer_lsa = xhost2sockaddr(argv[0], port);


	if(intf)
	{
		if(xx_read_interface(intf, &gifindex, NULL, NULL) < 0)
		{
			return -1;
		}
		if(gifindex > 0)
		{
			gPort = port;
		}

		if(dst_mac_str)
		{
			if(strlen(dst_mac_str) == 17)
			{
				mac_cnt = sscanf(dst_mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", 
								 &gDst_mac[0], &gDst_mac[1], &gDst_mac[2], &gDst_mac[3], &gDst_mac[4], &gDst_mac[5]);
				if(mac_cnt == 6)
				{
					gDest_arp = &gDst_mac[0];
				}
			}
		}
	}

#if ENABLE_DEBUG_TFTP
	fprintf(stderr, "using server '%s', remotefile '%s', localfile '%s'\n",
			xmalloc_sockaddr2dotted(&peer_lsa->sa),
			remotefile, localfile);
#endif

	result = tftp( USE_GETPUT(cmd,) peer_lsa, remotefile, fd, port, blocksize);

	if (ENABLE_FEATURE_CLEAN_UP)
		close(fd);
	if (result != EXIT_SUCCESS && !LONE_DASH(localfile) && CMD_GET(cmd)) {
		unlink(localfile);
	}
	
	if(check_cmd)
	{
		fprintf(stdout, "tftp: ");
		if (CMD_GET(cmd))
			fprintf(stdout, "get ... ");
		if (CMD_PUT(cmd))
			fprintf(stdout, "put ... ");
		if(result == EXIT_SUCCESS) 
			fprintf(stdout, "ok!\n");
		else
			fprintf(stdout, "fail!\n");
	}

	return result;
}

#endif /* ENABLE_FEATURE_TFTP_GET || ENABLE_FEATURE_TFTP_PUT */
