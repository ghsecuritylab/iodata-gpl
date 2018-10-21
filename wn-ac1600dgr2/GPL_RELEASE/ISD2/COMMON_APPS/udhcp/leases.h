/* leases.h */
#ifndef _LEASES_H
#define _LEASES_H

#define HOSTNAME_LEN 40
#define DEVNAME_LEN  20

struct dhcpOfferedAddr {
	uint8_t chaddr[16];
	uint32_t yiaddr;	/* network order */
	uint32_t expires;	/* host order */
	uint8_t hostname[HOSTNAME_LEN];
#if HAS_DHCPD_IF_INFOMATION_FUNCTION
        uint8_t ifName[DEVNAME_LEN];
#endif
};

extern uint8_t blank_chaddr[];

void clear_lease(uint8_t *chaddr, uint32_t yiaddr);
struct dhcpOfferedAddr *add_lease(uint8_t *chaddr, uint32_t yiaddr, unsigned long lease);
int lease_expired(struct dhcpOfferedAddr *lease);
struct dhcpOfferedAddr *oldest_expired_lease(void);
struct dhcpOfferedAddr *find_lease_by_chaddr(uint8_t *chaddr);
struct dhcpOfferedAddr *find_lease_by_yiaddr(uint32_t yiaddr);
uint32_t find_address(int check_expired);


#endif
