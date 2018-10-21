/* vi: set sw=4 ts=4: */
/*
 * files.c -- DHCP server file manipulation *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#ifdef UDHCP_WITHOUT_BUSYBOX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <netdb.h>
#endif
#include <netinet/ether.h>

#include "common.h"
#include "dhcpd.h"
#include "options.h"

#ifdef UDHCP_WITHOUT_BUSYBOX
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#endif

#define IFACE_UNKNOWN "unknown"

/* on these functions, make sure your datatype matches */
static int read_ip(const char *line, void *arg)
{
#ifdef UDHCP_WITHOUT_BUSYBOX
	struct in_addr *addr = arg;
	struct hostent *host;
	int retval = 1;

	if (!inet_aton(line, addr)) {
		if ((host = gethostbyname(line)))
			addr->s_addr = *((unsigned long *) host->h_addr_list[0]);
		else retval = 0;
	}
	return retval;
#else
	len_and_sockaddr *lsa;

	lsa = host_and_af2sockaddr(line, 0, AF_INET);
	if (!lsa)
		return 0;
	*(uint32_t*)arg = lsa->sin.sin_addr.s_addr;
	free(lsa);
	return 1;
#endif
}

static int read_mac(const char *line, void *arg)
{
	uint8_t *mac_bytes = arg;
	struct ether_addr *temp_ether_addr;

	temp_ether_addr = ether_aton(line);
	if (temp_ether_addr == NULL)
		return 0;
	memcpy(mac_bytes, temp_ether_addr, 6);
	return 1;
}


static int read_str(const char *line, void *arg)
{
	char **dest = arg;

	free(*dest);
	*dest = xstrdup(line);
	return 1;
}


static int read_u32(const char *line, void *arg)
{
#ifdef UDHCP_WITHOUT_BUSYBOX
	uint32_t *dest = arg;
	char *endptr;
	*dest = strtoul(line, &endptr, 0);
	return endptr[0] == '\0';
#else
	*(uint32_t*)arg = bb_strtou32(line, NULL, 10);
	return errno == 0;
#endif
}


static int read_yn(const char *line, void *arg)
{
	char *dest = arg;

	if (!strcasecmp("yes", line)) {
		*dest = 1;
		return 1;
	}
	if (!strcasecmp("no", line)) {
		*dest = 0;
		return 1;
	}
	return 0;
}


/* find option 'code' in opt_list */
struct option_set *find_option(struct option_set *opt_list, char code)
{
	while (opt_list && opt_list->data[OPT_CODE] < code)
		opt_list = opt_list->next;

	if (opt_list && opt_list->data[OPT_CODE] == code)
		return opt_list;
	return NULL;
}


/* add an option to the opt_list */
static void attach_option(struct option_set **opt_list,
		const struct dhcp_option *option, char *buffer, int length)
{
	struct option_set *existing, *new, **curr;

	existing = find_option(*opt_list, option->code);
	if (!existing) {
		DEBUG("Attaching option %s to list", option->name);

#if ENABLE_FEATURE_RFC3397
		if ((option->flags & TYPE_MASK) == OPTION_STR1035)
			/* reuse buffer and length for RFC1035-formatted string */
			buffer = dname_enc(NULL, 0, buffer, &length);
#endif

		/* make a new option */
		new = xmalloc(sizeof(*new));
		new->data = xmalloc(length + 2);
		new->data[OPT_CODE] = option->code;
		new->data[OPT_LEN] = length;
		memcpy(new->data + 2, buffer, length);

		curr = opt_list;
		while (*curr && (*curr)->data[OPT_CODE] < option->code)
			curr = &(*curr)->next;

		new->next = *curr;
		*curr = new;
#if ENABLE_FEATURE_RFC3397
		if ((option->flags & TYPE_MASK) == OPTION_STR1035 && buffer != NULL)
			free(buffer);
#endif
		return;
	}

	/* add it to an existing option */
	DEBUG("Attaching option %s to existing member of list", option->name);
	if (option->flags & OPTION_LIST) {
#if ENABLE_FEATURE_RFC3397
		if ((option->flags & TYPE_MASK) == OPTION_STR1035)
			/* reuse buffer and length for RFC1035-formatted string */
			buffer = dname_enc(existing->data + 2,
					existing->data[OPT_LEN], buffer, &length);
#endif
		if (existing->data[OPT_LEN] + length <= 255) {
			existing->data = xrealloc(existing->data,
					existing->data[OPT_LEN] + length + 3);
			if ((option->flags & TYPE_MASK) == OPTION_STRING) {
				/* ' ' can bring us to 256 - bad */
				if (existing->data[OPT_LEN] + length >= 255)
					return;
				/* add space separator between STRING options in a list */
				existing->data[existing->data[OPT_LEN] + 2] = ' ';
				existing->data[OPT_LEN]++;
			}
			memcpy(existing->data + existing->data[OPT_LEN] + 2, buffer, length);
			existing->data[OPT_LEN] += length;
		} /* else, ignore the data, we could put this in a second option in the future */
#if ENABLE_FEATURE_RFC3397
		if ((option->flags & TYPE_MASK) == OPTION_STR1035 && buffer != NULL)
			free(buffer);
#endif
	} /* else, ignore the new data */
}


/* read a dhcp option and add it to opt_list */
static int read_opt(const char *const_line, void *arg)
{
	struct option_set **opt_list = arg;
	char *opt, *val, *endptr;
	const struct dhcp_option *option;
	int retval = 0, length;
	char buffer[8];
	char *line;
	uint16_t *result_u16 = (uint16_t *) buffer;
	uint32_t *result_u32 = (uint32_t *) buffer;

	/* Cheat, the only const line we'll actually get is "" */
	line = (char *) const_line;
	opt = strtok(line, " \t=");
	if (!opt) return 0;

	option = dhcp_options;
	while (1) {
		if (!option->code)
			return 0;
		if (!strcasecmp(option->name, opt))
			break;
		option++;
	}

	do {
		val = strtok(NULL, ", \t");
		if (!val) break;
		length = option_lengths[option->flags & TYPE_MASK];
		retval = 0;
		opt = buffer; /* new meaning for variable opt */
		switch (option->flags & TYPE_MASK) {
		case OPTION_IP:
			retval = read_ip(val, buffer);
			break;
		case OPTION_IP_PAIR:
			retval = read_ip(val, buffer);
			val = strtok(NULL, ", \t/-");
			if (!val)
				retval = 0;
			if (retval)
				retval = read_ip(val, buffer + 4);
			break;
		case OPTION_STRING:
#if ENABLE_FEATURE_RFC3397
		case OPTION_STR1035:
#endif
			length = strlen(val);
			if(option->name && strcasecmp(option->name, "domain")==0 && strcasecmp(val, "defdomain")==0) {
				opt = '\0';
				retval = 1;
				continue;
			}
			if (length > 0) {
				if (length > 254) length = 254;
				opt = val;
				retval = 1;
			}
			break;
		case OPTION_BOOLEAN:
			retval = read_yn(val, buffer);
			break;
		case OPTION_U8:
			buffer[0] = strtoul(val, &endptr, 0);
			retval = (endptr[0] == '\0');
			break;
		/* htonX are macros in older libc's, using temp var
		 * in code below for safety */
		/* TODO: use bb_strtoX? */
		case OPTION_U16: {
			unsigned long tmp = strtoul(val, &endptr, 0);
			*result_u16 = htons(tmp);
			retval = (endptr[0] == '\0' /*&& tmp < 0x10000*/);
			break;
		}
		case OPTION_S16: {
			long tmp = strtol(val, &endptr, 0);
			*result_u16 = htons(tmp);
			retval = (endptr[0] == '\0');
			break;
		}
		case OPTION_U32: {
			unsigned long tmp = strtoul(val, &endptr, 0);
			*result_u32 = htonl(tmp);
			retval = (endptr[0] == '\0');
			break;
		}
		case OPTION_S32: {
			long tmp = strtol(val, &endptr, 0);
			*result_u32 = htonl(tmp);
			retval = (endptr[0] == '\0');
			break;
		}
		default:
			break;
		}
		if (retval)
			attach_option(opt_list, option, opt, length);
	} while (retval && option->flags & OPTION_LIST);
	return retval;
}

static int read_staticlease(const char *const_line, void *arg)
{
	char *line;
	char *mac_string;
	char *ip_string;
	uint8_t *mac_bytes;
	uint32_t *ip;

	/* Allocate memory for addresses */
	mac_bytes = xmalloc(sizeof(unsigned char) * 8);
	ip = xmalloc(sizeof(uint32_t));

	/* Read mac */
	line = (char *) const_line;
	mac_string = strtok(line, " \t");
	read_mac(mac_string, mac_bytes);

	/* Read ip */
	ip_string = strtok(NULL, " \t");
	read_ip(ip_string, ip);

	addStaticLease(arg, mac_bytes, ip);

	if (ENABLE_FEATURE_UDHCP_DEBUG) printStaticLeases(arg);

	return 1;
}


struct config_keyword {
	const char *keyword;
	int (*handler)(const char *line, void *var);
	void *var;
	const char *def;
};

static const struct config_keyword keywords[] = {
	/* keyword       handler   variable address               default */
	{"start",        read_ip,  &(server_config.start_ip),     "192.168.0.20"},
	{"end",          read_ip,  &(server_config.end_ip),       "192.168.0.254"},
	{"interface",    read_str, &(server_config.interface),    "eth0"},
	{"option",       read_opt, &(server_config.options),      ""},
	{"opt",          read_opt, &(server_config.options),      ""},
	/* Avoid "max_leases value not sane" warning by setting default
	 * to default_end_ip - default_start_ip + 1: */
	{"max_leases",   read_u32, &(server_config.max_leases),   "235"},
	{"remaining",    read_yn,  &(server_config.remaining),    "yes"},
	{"auto_time",    read_u32, &(server_config.auto_time),    "7200"},
	{"decline_time", read_u32, &(server_config.decline_time), "3600"},
	{"conflict_time",read_u32, &(server_config.conflict_time),"3600"},
	{"offer_time",   read_u32, &(server_config.offer_time),   "60"},
	{"min_lease",    read_u32, &(server_config.min_lease),    "60"},
	{"lease_file",   read_str, &(server_config.lease_file),   LEASES_FILE},
	{"pidfile",      read_str, &(server_config.pidfile),      "/var/run/udhcpd.pid"},
	{"notify_file",  read_str, &(server_config.notify_file),  ""},
	{"siaddr",       read_ip,  &(server_config.siaddr),       "0.0.0.0"},
	{"sname",        read_str, &(server_config.sname),        ""},
	{"boot_file",    read_str, &(server_config.boot_file),    ""},
	{"static_lease", read_staticlease, &(server_config.static_leases), ""},
#if HAS_USER_MANUAL_BOOTPFLAG	
	{"bootp_type",   read_u32,   &(server_config.bootp_type), "0"},
#endif	
	/* ADDME: static lease */
};


/*
 * Domain names may have 254 chars, and string options can be 254
 * chars long. However, 80 bytes will be enough for most, and won't
 * hog up memory. If you have a special application, change it
 */
#define READ_CONFIG_BUF_SIZE 80

int read_config(const char *file)
{
	FILE *in;
	char buffer[READ_CONFIG_BUF_SIZE], *token, *line;
	int i, lm = 0;

	for (i = 0; i < ARRAY_SIZE(keywords); i++)
		if (keywords[i].def[0])
			keywords[i].handler(keywords[i].def, keywords[i].var);

#ifdef UDHCP_WITHOUT_BUSYBOX
	if (!(in = fopen(file, "r"))) {
		LOG(LOG_ERR, "unable to open config file: %s", file);
		return 0;
	}
#else
	in = fopen_or_warn(file, "r");
	if (!in) {
		return 0;
	}
#endif

	while (fgets(buffer, READ_CONFIG_BUF_SIZE, in)) {
		char debug_orig[READ_CONFIG_BUF_SIZE];
		char *p;

		lm++;
		p = strchr(buffer, '\n');
		if (p) *p = '\0';
		if (ENABLE_FEATURE_UDHCP_DEBUG) strcpy(debug_orig, buffer);
		p = strchr(buffer, '#');
		if (p) *p = '\0';

		if (!(token = strtok(buffer, " \t"))) continue;
		if (!(line = strtok(NULL, ""))) continue;

		/* eat leading whitespace */
#ifdef UDHCP_WITHOUT_BUSYBOX
		line = line + strspn(line, " \t=");
#else
		line = skip_whitespace(line);
#endif
		/* eat trailing whitespace */
		i = strlen(line) - 1;
		while (i >= 0 && isspace(line[i]))
			line[i--] = '\0';

		for (i = 0; i < ARRAY_SIZE(keywords); i++)
			if (!strcasecmp(token, keywords[i].keyword))
				if (!keywords[i].handler(line, keywords[i].var)) {
					bb_error_msg("cannot parse line %d of %s", lm, file);
					if (ENABLE_FEATURE_UDHCP_DEBUG)
						bb_error_msg("cannot parse '%s'", debug_orig);
					/* reset back to the default value */
					keywords[i].handler(keywords[i].def, keywords[i].var);
				}
	}
	fclose(in);

	server_config.start_ip = ntohl(server_config.start_ip);
	server_config.end_ip = ntohl(server_config.end_ip);

	return 1;
}

#if HAS_DHCPD_IF_INFOMATION_FUNCTION
#if 0 //cfho 2008-0625, because we seldom run the dhcpd on ethx, but only br0, we do not need the following codes 

/* Read the ARP cache in the kernel. */
/* Called only from when the interface name is "ethXX" */
char *arp_find_if_by_mac(uint8_t *macAddr)
{
        const char *ifName;
        FILE *fp;
        int type, flags;
        int num;
        char ip[128];
        uint8_t chaddr[16];
        char mask[128];
        char line[128];
        char dev[128];
    
        if (macAddr==NULL) return NULL;

        fp = xfopen("/proc/net/arp", "r");
        /* Bypass header -- read one line */
        fgets(line, sizeof(line), fp);

        /* Read the ARP cache entries. */
        while (fgets(line, sizeof(line), fp)) {

                mask[0] = '-'; mask[1] = '\0';
                dev[0] = '-'; dev[1] = '\0';
                /* All these strings can't overflow
                 * because fgets above reads limited amount of data */
                //num = sscanf(line, "%s 0x%x 0x%x %s %s %s\n",
                //                       ip, &type, &flags, hwa, mask, dev);
                num = sscanf(line, "%s 0x%x 0x%x %02X:%02X:%02X:%02X:%02X:%02X %s %s\n",
                                         ip, &type, &flags, 
                                         &chaddr[0],&chaddr[1],&chaddr[2],
                                         &chaddr[3],&chaddr[4],&chaddr[5],
                                         mask, dev);

                if (chaddr[5]==macAddr[5] && chaddr[4]==macAddr[4] &&chaddr[3]==macAddr[3] &&
                    chaddr[2]==macAddr[2] && chaddr[1]==macAddr[1] &&chaddr[0]==macAddr[0])
                {
                    return (&dev[0]);
                }
        }
        fclose(fp);


        return 0;
}
#endif
#include <linux/if_bridge.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
/* Called only from when the interface name is "ethXX" */
/* cfho 2008-0625, port from bridge utils 1.2 */
#define CHUNK 128
struct fdb_entry
{
        u_int8_t mac_addr[6];
        u_int16_t port_no;
        unsigned char is_local;
        struct timeval ageing_timer_value;
};

static inline void __copy_fdb(struct fdb_entry *ent, 
                              const struct __fdb_entry *f)
{
        memcpy(ent->mac_addr, f->mac_addr, 6);
        ent->port_no = f->port_no;
        ent->is_local = f->is_local;
 //       __jiffies_to_tv(&ent->ageing_timer_value, f->ageing_timer_value);
}

int br_read_fdb(const char *bridge, struct fdb_entry *fdbs, 
                unsigned long offset, int num, int br_socket_fd)
{
        int i, n;
        struct __fdb_entry fe[num];
        
        /* old kernel, use ioctl */
        unsigned long args[4] = { BRCTL_GET_FDB_ENTRIES,
                                          (unsigned long) fe,
                                          num, offset };
        struct ifreq ifr;
        int retries = 0;


        strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
        ifr.ifr_data = (char *) args;

retry:
        n = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

        /* table can change during ioctl processing */
        if (n < 0 && errno == EAGAIN && ++retries < 10) {
            sleep(0);
            goto retry;
        }
    
        for (i = 0; i < n; i++) 
                __copy_fdb(fdbs+i, fe+i);

        return n;
}

char* brtable_find_if_by_mac(unsigned char *macAddr)
{
#define MAX_PORTS 1024
#define CHUNK 128
#define BRG_DEV "br0"
	int br_socket_fd = -1;
	int i, n, err;
	struct fdb_entry *fdb = NULL;
	int offset = 0;
	static char ifname[IFNAMSIZ];
	struct ifreq ifr;
	int ifindices[MAX_PORTS];
	unsigned long args[4] = { BRCTL_GET_PORT_LIST, (unsigned long)ifindices, MAX_PORTS, 0 };
	int port = 0;
    char *lanIface=server_config.interface;

#if 0 // Debug
	FILE *fp;

	fp = xfopen("/br.txt", "wt");
	fprintf(fp, "=== %s ====\n", __FUNCTION__);
#endif

	if((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		goto FIND_IF_FAIL;

	for(;;) 
	{
		fdb = realloc(fdb, (offset + CHUNK) * sizeof(struct fdb_entry));
		if (!fdb) {
			fprintf(stderr, "Out of memory\n");
			goto FIND_IF_FAIL;
		}

		n = br_read_fdb(lanIface, fdb+offset, offset, CHUNK, br_socket_fd);
		if (n == 0)
			break;

		if (n < 0) {
			fprintf(stderr, "read of forward table failed: %s\n",
					strerror(errno));
			goto FIND_IF_FAIL;
		}

		offset += n;
	}

#if 0 // Debug
	fprintf(fp,"port no\tmac addr\t\tis local?\tageing timer\n");
	for (i = 0; i < offset; i++) 
	{
		const struct fdb_entry *f = fdb + i;
		fprintf(fp,"%3i\t", f->port_no);
		fprintf(fp,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\t",
				f->mac_addr[0], f->mac_addr[1], f->mac_addr[2],
				f->mac_addr[3], f->mac_addr[4], f->mac_addr[5]);
		fprintf(fp,"%s\t\t", f->is_local?"yes":"no");
		//br_show_timer(&f->ageing_timer_value);
		fprintf(fp,"\n");
	}
	fclose(fp);
#endif

	for (i = 0; i < offset; i++) 
	{
		const struct fdb_entry *f = fdb + i;

		if (f->mac_addr[5]==macAddr[5] && f->mac_addr[4]==macAddr[4] &&f->mac_addr[3]==macAddr[3] &&
			f->mac_addr[2]==macAddr[2] && f->mac_addr[1]==macAddr[1] &&f->mac_addr[0]==macAddr[0])
		{
			port = f->port_no;
			break;
		}
	}

	memset(ifindices, 0, sizeof(ifindices));
	strncpy(ifr.ifr_name, lanIface, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;

	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	if (err < 0)
	{
		fprintf(stderr, "list ports for bridge:'%s' failed: %s\n", 
				lanIface, strerror(errno));
		goto FIND_IF_FAIL;
	}

	for (i = 0; i < MAX_PORTS; i++)
	{
		if (!ifindices[i])
			continue;

		if (if_indextoname(ifindices[i], ifname)) 
		{
			if (i==port)
			{
				goto FIND_IF_END;
			}
		}
	}

FIND_IF_FAIL:
	strcpy(ifname, IFACE_UNKNOWN);

FIND_IF_END:
	if(br_socket_fd >= 0)
	{
		close(br_socket_fd);
	}
	if(fdb)
	{
		free(fdb);
	}

	return (&ifname[0]);
}
#endif // HAS_DHCPD_IF_INFOMATION_FUNCTION //
void write_leases(void)
{
#ifdef UDHCP_WITHOUT_BUSYBOX
	FILE *fp;
	char buf[255];
#else
	int fp;
#endif
	unsigned i;
#if 1 //cfho 2008-0109
    time_t curr = monotonic_sec();
#else
	time_t curr = time(0);
#endif
	unsigned long tmp_time;

#ifdef UDHCP_WITHOUT_BUSYBOX
	if (!(fp = fopen(server_config.lease_file, "w"))) {
		LOG(LOG_ERR, "Unable to open %s for writing", server_config.lease_file);
		return;
	}
#else
	fp = open3_or_warn(server_config.lease_file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if (fp < 0) {
		return;
	}
#endif

	for (i = 0; i < server_config.max_leases; i++) {
		if (leases[i].yiaddr != 0) {

			/* screw with the time in the struct, for easier writing */
			tmp_time = leases[i].expires;

			if (server_config.remaining) {
				if (lease_expired(&(leases[i])))
					leases[i].expires = 0;
				else leases[i].expires -= curr;
			} /* else stick with the time we got */
			leases[i].expires = htonl(leases[i].expires);

#if HAS_DHCPD_IF_INFOMATION_FUNCTION
			if(0==strlen(leases[i].ifName)||(0==strcmp(IFACE_UNKNOWN, leases[i].ifName)))
			{
				strcpy(leases[i].ifName, brtable_find_if_by_mac(&(leases[i].chaddr[0])));
			}
#endif    
			// FIXME: error check??
#ifdef UDHCP_WITHOUT_BUSYBOX
			fwrite(&leases[i], sizeof(struct dhcpOfferedAddr), 1, fp);
#else
			full_write(fp, &leases[i], sizeof(leases[i]));
#endif

			/* then restore it when done */
			leases[i].expires = tmp_time;
		}
	}
#ifdef UDHCP_WITHOUT_BUSYBOX
	fclose(fp);
#else
	close(fp);
#endif

	if (server_config.notify_file && strlen(server_config.notify_file)) {
#ifdef UDHCP_WITHOUT_BUSYBOX
		sprintf(buf, "%s -f %s", server_config.notify_file, server_config.lease_file);
		system(buf);
#else
		char *cmd = xasprintf("%s %s", server_config.notify_file, server_config.lease_file);
		system(cmd);
		free(cmd);
#endif
	}
}


void read_leases(const char *file)
{
#ifdef UDHCP_WITHOUT_BUSYBOX
	FILE *fp;
#else
	int fp;
#endif
	unsigned int i = 0;
	struct dhcpOfferedAddr lease, *please;

#ifdef UDHCP_WITHOUT_BUSYBOX
	if (!(fp = fopen(file, "r"))) {
		LOG(LOG_ERR, "Unable to open %s for reading", file);
		return;
	}
#else
	fp = open_or_warn(file, O_RDONLY);
	if (fp < 0) {
		return;
	}
#endif

#ifdef UDHCP_WITHOUT_BUSYBOX
	while (i < server_config.max_leases && (fread(&lease, sizeof lease, 1, fp) == 1)) {
#else
	while (i < server_config.max_leases
	 && full_read(fp, &lease, sizeof(lease)) == sizeof(lease)
	) {
#endif

		/* ADDME: is it a static lease */
		uint32_t y = ntohl(lease.yiaddr);
		if (y >= server_config.start_ip && y <= server_config.end_ip) {
			lease.expires = ntohl(lease.expires);
#if 1 //cfho 2008-0109
            if (!server_config.remaining)
                lease.expires -= monotonic_sec();
#else
			if (!server_config.remaining)
				lease.expires -= time(0);
#endif
		    please = add_lease(lease.chaddr, lease.yiaddr, lease.expires, lease.hostname);
			if (!please) {
				bb_error_msg("too many leases while loading %s", file);
				break;
			}
#if HAS_DHCPD_IF_INFOMATION_FUNCTION
            else
            {
	            memcpy(please->ifName, lease.ifName, DEVNAME_LEN);
	        }
#endif
			i++;
		}
	}
	DEBUG("Read %d leases", i);
#ifdef UDHCP_WITHOUT_BUSYBOX
	fclose(fp);
#else
	close(fp);
#endif
}
