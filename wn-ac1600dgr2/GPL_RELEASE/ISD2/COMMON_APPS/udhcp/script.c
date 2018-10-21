/* script.c
 *
 * Functions to call the DHCP client notification scripts
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

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "options.h"
#include "dhcpd.h"
#include "dhcpc.h"
#include "common.h"

extern int key_index;
extern int fixLocal4thIP;

#if 0 //DEBUG
#define SYSTEM(x) printf(x)
#else
#define SYSTEM(x) { printf(x); printf("\n"); system(x); }
#endif

/* get a rough idea of how long an option will be (rounding up...) */
static const int max_option_length[] = {
	[OPTION_IP] =		sizeof("255.255.255.255 "),
	[OPTION_IP_PAIR] =	sizeof("255.255.255.255 ") * 2,
	[OPTION_STRING] =	1,
	[OPTION_BOOLEAN] =	sizeof("yes "),
	[OPTION_U8] =		sizeof("255 "),
	[OPTION_U16] =		sizeof("65535 "),
	[OPTION_S16] =		sizeof("-32768 "),
	[OPTION_U32] =		sizeof("4294967295 "),
	[OPTION_S32] =		sizeof("-2147483684 "),
#if SUPPORT_IPV6_6RD
	[OPTION_6RD] =		sizeof("32 128 FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF 255.255.255.255 "),
#endif
};


static inline int upper_length(int length, int opt_index)
{
	return max_option_length[opt_index] *
		(length / option_lengths[opt_index]);
}

#if DHCP_CLIENT_USE_SCRIPT_FILE

static int sprintip(char *dest, char *pre, uint8_t *ip)
{
	return sprintf(dest, "%s%d.%d.%d.%d", pre, ip[0], ip[1], ip[2], ip[3]);
}


/* really simple implementation, just count the bits */
static int mton(struct in_addr *mask)
{
	int i;
	unsigned long bits = ntohl(mask->s_addr);
	/* too bad one can't check the carry bit, etc in c bit
	 * shifting */
	for (i = 0; i < 32 && !((bits >> i) & 1); i++);
	return 32 - i;
}


/* Fill dest with the text of option 'option'. */
static void fill_options(char *dest, uint8_t *option, struct dhcp_option *type_p)
{
	int type, optlen;
	uint16_t val_u16;
	int16_t val_s16;
	uint32_t val_u32;
	int32_t val_s32;
	int len = option[OPT_LEN - 2];

	dest += sprintf(dest, "%s=", type_p->name);

	type = type_p->flags & TYPE_MASK;
	optlen = option_lengths[type];
	for(;;) {
		switch (type) {
		case OPTION_IP_PAIR:
			dest += sprintip(dest, "", option);
			*(dest++) = '/';
			option += 4;
			optlen = 4;
		case OPTION_IP:	/* Works regardless of host byte order. */
			dest += sprintip(dest, "", option);
 			break;
		case OPTION_BOOLEAN:
			dest += sprintf(dest, *option ? "yes" : "no");
			break;
		case OPTION_U8:
			dest += sprintf(dest, "%u", *option);
			break;
		case OPTION_U16:
			memcpy(&val_u16, option, 2);
			dest += sprintf(dest, "%u", ntohs(val_u16));
			break;
		case OPTION_S16:
			memcpy(&val_s16, option, 2);
			dest += sprintf(dest, "%d", ntohs(val_s16));
			break;
		case OPTION_U32:
			memcpy(&val_u32, option, 4);
			dest += sprintf(dest, "%lu", (unsigned long) ntohl(val_u32));
			break;
		case OPTION_S32:
			memcpy(&val_s32, option, 4);
			dest += sprintf(dest, "%ld", (long) ntohl(val_s32));
			break;
		case OPTION_STRING:
			memcpy(dest, option, len);
			dest[len] = '\0';
			return;	 /* Short circuit this case */
		}
		option += optlen;
		len -= optlen;
		if (len <= 0) break;
		dest += sprintf(dest, " ");
	}
}

/* put all the parameters into an environment */
static char **fill_envp(struct dhcpMessage *packet)
{
	int num_options = 0;
	int i, j;
	char **envp;
	uint8_t *temp;
	struct in_addr subnet;
	char over = 0;

	if (packet == NULL)
		num_options = 0;
	else {
		for (i = 0; dhcp_options[i].code; i++)
			if (get_option(packet, dhcp_options[i].code)) {
				num_options++;
				if (dhcp_options[i].code == DHCP_SUBNET)
					num_options++; /* for mton */
			}
		if (packet->siaddr) num_options++;
		if ((temp = get_option(packet, DHCP_OPTION_OVER)))
			over = *temp;
		if (!(over & FILE_FIELD) && packet->file[0]) num_options++;
		if (!(over & SNAME_FIELD) && packet->sname[0]) num_options++;
	}

	envp = xcalloc(sizeof(char *), num_options + 5);
	j = 0;
	asprintf(&envp[j++], "interface=%s", client_config.interface);
	asprintf(&envp[j++], "%s=%s", "PATH",
		getenv("PATH") ? : "/bin:/usr/bin:/sbin:/usr/sbin");
	asprintf(&envp[j++], "%s=%s", "HOME", getenv("HOME") ? : "/");

	if (packet == NULL) return envp;

	envp[j] = xmalloc(sizeof("ip=255.255.255.255"));
	sprintip(envp[j++], "ip=", (uint8_t *) &packet->yiaddr);


	for (i = 0; dhcp_options[i].code; i++) {
		if (!(temp = get_option(packet, dhcp_options[i].code)))
			continue;
		envp[j] = xmalloc(upper_length(temp[OPT_LEN - 2],
			dhcp_options[i].flags & TYPE_MASK) + strlen(dhcp_options[i].name) + 2);
		fill_options(envp[j++], temp, &dhcp_options[i]);

		/* Fill in a subnet bits option for things like /24 */
		if (dhcp_options[i].code == DHCP_SUBNET) {
			memcpy(&subnet, temp, 4);
			asprintf(&envp[j++], "mask=%d", mton(&subnet));
		}
	}
	if (packet->siaddr) {
		envp[j] = xmalloc(sizeof("siaddr=255.255.255.255"));
		sprintip(envp[j++], "siaddr=", (uint8_t *) &packet->siaddr);
	}
	if (!(over & FILE_FIELD) && packet->file[0]) {
		/* watch out for invalid packets */
		packet->file[sizeof(packet->file) - 1] = '\0';
		asprintf(&envp[j++], "boot_file=%s", packet->file);
	}
	if (!(over & SNAME_FIELD) && packet->sname[0]) {
		/* watch out for invalid packets */
		packet->sname[sizeof(packet->sname) - 1] = '\0';
		asprintf(&envp[j++], "sname=%s", packet->sname);
	}
	return envp;
}
#define UDHCPC_SCRIPT   "/etc/udhcpc/udhcpc.%s"
static int isFileExisted(const char *filename)
{
	int rval;

	rval = access(filename, F_OK);

	return rval ? 0 : 1;
}

/* Call a script with a par file and env vars */
void run_script(struct dhcpMessage *packet, const char *name)
{
	int pid;
	char **envp, **curr;
	char filename[80];

	if (client_config.script == NULL)
		return;

	/*Adonn 08-0116: if script file doesn't exist, will not execute script*/
	sprintf(filename, UDHCPC_SCRIPT, name);
	if(!isFileExisted(filename))
	{
		return;
	}
	DEBUG(LOG_INFO, "vforking and execle'ing %s", client_config.script);

	envp = fill_envp(packet);
	/* call script */
	pid = vfork();
	if (pid) {
		waitpid(pid, NULL, 0);
		for (curr = envp; *curr; curr++) free(*curr);
		free(envp);
		return;
	} else if (pid == 0) {
		/* close fd's? */

		/* exec script */
		execle(client_config.script, client_config.script,
		       name, NULL, envp);
		LOG(LOG_ERR, "script %s failed: %m", client_config.script);
		exit(1);
	}
}

#else /* use sysconfd to control */
void run_script(struct dhcpMessage *packet, const char *name)
{
#define RESOLV_CONF "/etc/resolv.conf"
#define DHCP_6RD_INFO "/etc/dhcp_6rd_info.txt"
#define DHCP_MS_CLASSLESS_STATIC_ROUTE_ADD_INFO "/etc/dhcp_option249_add_info.txt"
#define DHCP_MS_CLASSLESS_STATIC_ROUTE_DEL_INFO "/etc/dhcp_option249_del_info.txt"
/*
#!/bin/sh

RESOLV_CONF="/etc/resolv.conf"

[ -n "$broadcast" ] && BROADCAST="broadcast $broadcast"
[ -n "$subnet" ] && NETMASK="netmask $subnet"

/sbin/ifconfig $interface $ip $BROADCAST $NETMASK

if [ -n "$router" ]
then
	echo "deleting routers"
	while /sbin/route del default gw 0.0.0.0 dev $interface
	do :
	done

	for i in $router
	do
		/sbin/route add default gw $i dev $interface
	done
fi

echo -n > $RESOLV_CONF
[ -n "$domain" ] && echo domain $domain >> $RESOLV_CONF
for i in $dns
do
	echo adding dns $i
	echo nameserver $i >> $RESOLV_CONF
done

*/
    uint8_t *message;
    char commands[1000];
    char fname[50];
	char dhcp_domain_name[256]={0};
    unsigned char *pdata = NULL;
    unsigned int broadcast_ip = 0;
    unsigned int subnet_ip = 0;
    unsigned int local_ip = 0;
    unsigned int router_ip = 0;
    unsigned int dns_ip = 0;
    unsigned char len;
    int i=0;    
    FILE *f;
    static  int tmp_router_ip = 0;

/*
    char **envp, **curr;
    envp = fill_envp(packet);
    
    for (curr = envp; *curr; curr++)
    {
        printf("curr = %s\n", *curr);
        free(*curr);
    }
    free(*envp);
*/

    /* deconfig */
	if(strcmp(name, "deconfig") == 0)
	{
		/* #!/bin/sh
		   /sbin/ifconfig $interface 0.0.0.0 */
		sprintf(commands, "/sbin/ifconfig %s 0.0.0.0",  client_config.interface);
        system(commands);
	}

    /* [ -n "$broadcast" ] && BROADCAST="broadcast $broadcast" */
    if(packet==NULL)
        return;

	//20130411 Jason: get dhcp domain name for IODATA request
    message = get_option(packet, DHCP_DOMAIN_NAME);
    if(message)
    {
		len = *(unsigned char*)(message-1);
		sprintf(dhcp_domain_name, "%s", message);
		dhcp_domain_name[len]='\0';

        //2017/04/11 Jason: Fix shell script attack for JPCERT.
        if(strchr(dhcp_domain_name, '`') != NULL)
        {
            //do nothing...
        }
        else
        {
            sprintf(commands, "echo '%s' > /tmp/dhcp_domain_name", dhcp_domain_name);
            SYSTEM(commands);
        }
	}

    message = get_option(packet, DHCP_BROADCAST);
    if(message) 
        broadcast_ip = *(unsigned int*)message;
    
    /* [ -n "$subnet" ] && NETMASK="netmask $subnet" */
    message = get_option(packet, DHCP_SUBNET);
    if(message)
        subnet_ip = *(unsigned int*)message;
    
    /* /sbin/ifconfig $interface $ip $BROADCAST $NETMASK */
    local_ip = packet->yiaddr;
    if(local_ip)
    {
        pdata = (unsigned char *)&local_ip;
		if(fixLocal4thIP > 0)
		{
			sprintf(commands, "ifconfig %s %d.%d.%d.%d", client_config.interface, pdata[0], pdata[1], pdata[2], fixLocal4thIP);
		}
		else
		{
			sprintf(commands, "ifconfig %s %d.%d.%d.%d", client_config.interface, pdata[0], pdata[1], pdata[2], pdata[3]);
		}
        
        if(broadcast_ip)
        {
            pdata = (unsigned char *)&broadcast_ip;
            sprintf(commands, "%s broadcast %d.%d.%d.%d", commands, pdata[0], pdata[1], pdata[2], pdata[3]);
        }
        if(subnet_ip)
        {
            pdata = (unsigned char *)&subnet_ip;
            sprintf(commands, "%s netmask %d.%d.%d.%d", commands, pdata[0], pdata[1], pdata[2], pdata[3]);
        }
        printf("%s\n", commands);
        SYSTEM(commands);
    }


/*

if [ -n "$router" ]
then
	echo "deleting routers"
	while /sbin/route del default gw 0.0.0.0 dev $interface
	do :
	done

	for i in $router
	do
		/sbin/route add default gw $i dev $interface
	done
fi

*/

    message = get_option(packet, DHCP_ROUTER);
    if(message)
    {
        len = *(unsigned char*)(message-1);
        
        i=0;
        //for(i=0; i<len;i+=4)
        {
            router_ip = *(unsigned int*)(message+i);
            pdata = (unsigned char *)&router_ip;
            printf("len = %d, router = %d.%d.%d.%d\n", len, pdata[0], pdata[1], pdata[2], pdata[3]);
            sprintf(commands, "sysconf_cli 'setDefaultGateway %d %d.%d.%d.%d'", key_index, pdata[0], pdata[1], pdata[2], pdata[3]);
            SYSTEM(commands);
        }
    }


/*
echo -n > $RESOLV_CONF
[ -n "$domain" ] && echo domain $domain >> $RESOLV_CONF
for i in $dns
do
	echo adding dns $i
	echo nameserver $i >> $RESOLV_CONF
done
*/
    
    /* skip DNS for simple script */
    if(0==strcmp(name, "simple"))
    {
        if(tmp_router_ip == (int)router_ip)
        {
            return;
        }
        tmp_router_ip = router_ip;
    }

    message = get_option(packet, DHCP_DNS_SERVER);
    if(message)
    {
        len = *(unsigned char*)(message-1);
#if HAS_MULTIPLE_WAN
		if(key_index < 0) /* LAN udhcp client */
		{
			sprintf(fname, "%s", RESOLV_CONF);
		}
		else
		{
			sprintf(fname, "%s.%d", RESOLV_CONF, key_index);
		}
#else
        sprintf(fname, "%s", RESOLV_CONF);
#endif
        f = fopen(fname, "w");
        if(f != NULL)
        {
            for(i=0; i<len;i+=4)
            {
                dns_ip = *(unsigned int*)(message+i);
                pdata = (unsigned char *)&dns_ip;
                printf("len = %d, dns = %d.%d.%d.%d\n", len, pdata[0], pdata[1], pdata[2], pdata[3]);
                fprintf(f, "nameserver %d.%d.%d.%d\n", pdata[0], pdata[1], pdata[2], pdata[3]);
            }
            fclose(f);
        }
        else
        {
            printf("Fail to open %s\n", RESOLV_CONF);
        }
        
        //sprintf(commands, "sysconf_cli inform dns-Obtained  %d %d.%d.%d.%d", key_index, pdata[0], pdata[1], pdata[2], pdata[3]);
        //SYSTEM(commands);
    }
#if SUPPORT_IPV6_6RD
    /* Option binary format:
	 * 0                   1                   2                   3
	 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * |  OPTION_6RD   | option-length |  IPv4MaskLen  |  6rdPrefixLen |
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * |															   |
	 * |						   6rdPrefix						   |
	 * |						  (16 octets)						   |
	 * |															   |
	 * |															   |
	 * |															   |
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * |					 6rdBRIPv4Address(es)					   |
	 * .															   .
	 * .															   .
	 * .															   .
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *
	 * We convert it to a string "IPv4MaskLen 6rdPrefixLen 6rdPrefix 6rdBRIPv4Address"
	 */

	/* Sanity check: ensure that our length is at least 22 bytes, that
	 * IPv4MaskLen is <= 32, 6rdPrefixLen <= 128 and that the sum of
	 * (32 - IPv4MaskLen) + 6rdPrefixLen is less than or equal to 128.
	 * If any of these requirements is not fulfilled, return with empty
	 * value.
	 */
	if(client_config.option_6rd_enable)
	{
		unsigned int bripv4 = 0;

		message = get_option(packet, DHCP_6RD);
		if(message)
		{
			len = *(unsigned char*)(message-1);
			if ((len >= 22) && (*message <= 32) && (*(message+1) <= 128) &&
				(((32 - *message) + *(message+1)) <= 128))
			{
				if ( (f = fopen(DHCP_6RD_INFO, "w")) != NULL )
				{
					/* IPv4MaskLen */
					fprintf(f,"MaskLen %d\n",*message++);
					len--;
						
					/* 6rdPrefixLen */
					fprintf(f,"PrefixLen %d\n",*message++);
					len--;
						
					/* 6rdPrefix */
					fprintf(f,"6rdPrefix ");
					for (i=0;i<8;i+=2)
					{
						fprintf(f,"%02x%02x:",*(message+i),*(message+i+1));
					}
					fprintf(f,":\n");
					message += 16;
					len -= 16;

					/* 6rdBRIPv4Address */
					while(len >= 4)
					{
						bripv4 = *(unsigned int*)(message);
						pdata = (unsigned char *)&bripv4;
						printf("bripv4 = %d.%d.%d.%d\n", pdata[0], pdata[1], pdata[2], pdata[3]);
						fprintf(f,"6rdBRIPv4Address %d.%d.%d.%d\n", pdata[0], pdata[1], pdata[2], pdata[3]);
						message += 4;
						len -= 4;
		
						/* the code to determine the option size fails to work with
						 * lengths that art not a multiple of the minimum length,
						 * adding all advertised 6rdBRIPv4Address here would
						 * overflow the destination buffer, therefore skip the rest
						 * for now
						 */
						break;
					}
					fclose(f);
				}
				else
				{
					printf("Fail to open %s\n", DHCP_6RD_INFO);
				}

				SYSTEM("echo 1 > /etc/dhcp_6rd_connected");
			}
			else
			{
				SYSTEM("echo 0 > /etc/dhcp_6rd_connected");
			}
		}
		else
		{
			SYSTEM("echo 0 > /etc/dhcp_6rd_connected");
		}
	
		SYSTEM("sysconf_cli inform update_6rd_info");
	}
#endif
    /* DHCP option 249: Microsoft Classless Static Route Option */
    unsigned int flag_tmp=0, des_ip, netmask, gw;
    unsigned char mask_num = 0;
    FILE *f_add, *f_del;
    message = get_option(packet, DHCP_MS_CLASSLESS_STATIC_ROUTE);

    if(client_config.classless_static_route_enable == 0)
    {
        return;
    }
    sprintf(commands,"sysconf_cli inform dhcp_option_249_info del");
	SYSTEM(commands);
    if(message)
    {
        len = *(unsigned char*)(message-1);
        printf("DHCP ACK with MS_DHCP_STATIC_ROUTE (option 249), len=%d\n",len);

        if(len >= 5)    //rfc specifies minimun length is 5
        {
            if ( (f_add = fopen(DHCP_MS_CLASSLESS_STATIC_ROUTE_ADD_INFO, "w")) != NULL )
			{
                if ( (f_del = fopen(DHCP_MS_CLASSLESS_STATIC_ROUTE_DEL_INFO, "w")) != NULL )
                {
                    while(flag_tmp < len)
                    {
                        mask_num = message[flag_tmp];
                        if(mask_num == 0)
                        {
                            /* netmask is 0.0.0.0, default gw */
                            ((unsigned char *)&des_ip)[0] = ((unsigned char *)&des_ip)[1] = ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
                            ((unsigned char *)&gw)[0] = message[flag_tmp+1];
                            ((unsigned char *)&gw)[1] = message[flag_tmp+2];
                            ((unsigned char *)&gw)[2] = message[flag_tmp+3];
                            ((unsigned char *)&gw)[3] = message[flag_tmp+4];
                            flag_tmp = flag_tmp + 5;
                        }
                        else if(1 < mask_num && mask_num <= 8)
                        {
                            ((unsigned char *)&des_ip)[0] = message[flag_tmp+1];
                            ((unsigned char *)&des_ip)[1] = ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
                            ((unsigned char *)&gw)[0] = message[flag_tmp+2];
                            ((unsigned char *)&gw)[1] = message[flag_tmp+3];
                            ((unsigned char *)&gw)[2] = message[flag_tmp+4];
                            ((unsigned char *)&gw)[3] = message[flag_tmp+5];
                            flag_tmp = flag_tmp + 6;
                        }
                        else if(9 <= mask_num && mask_num <= 16)
                        {
                            ((unsigned char *)&des_ip)[0] = message[flag_tmp+1];
                            ((unsigned char *)&des_ip)[1] = message[flag_tmp+2];
                            ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
                            ((unsigned char *)&gw)[0] = message[flag_tmp+3];
                            ((unsigned char *)&gw)[1] = message[flag_tmp+4];
                            ((unsigned char *)&gw)[2] = message[flag_tmp+5];
                            ((unsigned char *)&gw)[3] = message[flag_tmp+6];
                            flag_tmp = flag_tmp + 7;
                        }
                        else if(17 <= mask_num && mask_num <= 24)
                        {
                            ((unsigned char *)&des_ip)[0] = message[flag_tmp+1];
                            ((unsigned char *)&des_ip)[1] = message[flag_tmp+2];
                            ((unsigned char *)&des_ip)[2] = message[flag_tmp+3];
                            ((unsigned char *)&des_ip)[3] = 0;
                            ((unsigned char *)&gw)[0] = message[flag_tmp+4];
                            ((unsigned char *)&gw)[1] = message[flag_tmp+5];
                            ((unsigned char *)&gw)[2] = message[flag_tmp+6];
                            ((unsigned char *)&gw)[3] = message[flag_tmp+7];
                            flag_tmp = flag_tmp + 8;
                        }
                        else if(25 <=mask_num && mask_num <= 32)
                        {
                            ((unsigned char *)&des_ip)[0] = message[flag_tmp+1];
                            ((unsigned char *)&des_ip)[1] = message[flag_tmp+2];
                            ((unsigned char *)&des_ip)[2] = message[flag_tmp+3];
                            ((unsigned char *)&des_ip)[3] = message[flag_tmp+4];
                            ((unsigned char *)&gw)[0] = message[flag_tmp+5];
                            ((unsigned char *)&gw)[1] = message[flag_tmp+6];
                            ((unsigned char *)&gw)[2] = message[flag_tmp+7];
                            ((unsigned char *)&gw)[3] = message[flag_tmp+8];
                            flag_tmp = flag_tmp + 9;
                        }
                        else
                        {
                            printf("%s, Unknown mask num %u\n", __FUNCTION__, mask_num);
                            continue;
                        }

                        memset(&netmask, 0, sizeof(unsigned int));
                        for(i=0;i<mask_num;++i)
                        {
                            netmask = netmask | (1<<i);
                        }

                        printf("Adding MS_DHCP_STATIC_ROUTE -net %u.%u.%u.%u netmask[%u] %u.%u.%u.%u router %u.%u.%u.%u metric 1\n",
                               ((unsigned char *)&des_ip)[0], ((unsigned char *)&des_ip)[1],
                               ((unsigned char *)&des_ip)[2], ((unsigned char *)&des_ip)[3],
                               mask_num,
                               ((unsigned char *)&netmask)[0], ((unsigned char *)&netmask)[1],
                               ((unsigned char *)&netmask)[2], ((unsigned char *)&netmask)[3],
                               ((unsigned char *)&gw)[0], ((unsigned char *)&gw)[1],
                               ((unsigned char *)&gw)[2], ((unsigned char *)&gw)[3]);
                        fprintf(f_add,"route add -net %u.%u.%u.%u netmask %u.%u.%u.%u gw %u.%u.%u.%u metric 1 dev %s\n",
                                ((unsigned char *)&des_ip)[0], ((unsigned char *)&des_ip)[1],
                                ((unsigned char *)&des_ip)[2], ((unsigned char *)&des_ip)[3],
                                ((unsigned char *)&netmask)[0], ((unsigned char *)&netmask)[1],
                                ((unsigned char *)&netmask)[2], ((unsigned char *)&netmask)[3],
                                ((unsigned char *)&gw)[0], ((unsigned char *)&gw)[1],
                                ((unsigned char *)&gw)[2], ((unsigned char *)&gw)[3],
								client_config.interface);
                        fprintf(f_del,"route del -net %u.%u.%u.%u netmask %u.%u.%u.%u gw %u.%u.%u.%u metric 1 dev %s\n",
                                ((unsigned char *)&des_ip)[0], ((unsigned char *)&des_ip)[1],
                                ((unsigned char *)&des_ip)[2], ((unsigned char *)&des_ip)[3],
                                ((unsigned char *)&netmask)[0], ((unsigned char *)&netmask)[1],
                                ((unsigned char *)&netmask)[2], ((unsigned char *)&netmask)[3],
                                ((unsigned char *)&gw)[0], ((unsigned char *)&gw)[1],
                                ((unsigned char *)&gw)[2], ((unsigned char *)&gw)[3],
								client_config.interface);
                    }
                }
                else
                {
                    printf("%s, can't open %s with write permission\n", __FUNCTION__, DHCP_MS_CLASSLESS_STATIC_ROUTE_DEL_INFO);
                    fclose(f_add);
                }
                fclose(f_add);
                fclose(f_del);
            }
            else
			{
				printf("%s, can't open %s with write permission\n", __FUNCTION__, DHCP_MS_CLASSLESS_STATIC_ROUTE_ADD_INFO);
			}
        }
        else
        {
            printf("%s, Invalid option length %u!\n", __FUNCTION__, len);
            return;
        }
        sprintf(commands,"sysconf_cli inform dhcp_option_249_info add");
		SYSTEM(commands);
    }

    return;
}
    
#endif
