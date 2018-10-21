/*****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project : SI-688
;    Creator : 
;    File    : sysMii.c
;    Abstract: API for read MII status of ethernet device
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;       cfho            2007-0104       Newly Create
;*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "checkLink.h"
// #include <sysUtil.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#if TARGET
#include "mii.h"
#include <gconfig.h>

#include <errno.h> 
static int skfd = -1;       /* AF_INET socket for ioctl() calls. */
static struct ifreq ifr;
/******************************************************/
#if 1 /* John Chang, 2007/11/01, IC+ MII code */
#if HAS_WAN_WIMAX
#define WAN_IFNAME     "eth0"
#elif FOR_ATHEROS_PLATFORM || FOR_UBICOM_PLATFORM
#define WAN_IFNAME     WAN_DEV
#else
#define WAN_IFNAME     "eth2"  /* IC+ Interhace */ 
#endif

#if 0 
static int mdio_read(int skfd, int location)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    mii->reg_num = location;
    if (ioctl(skfd, SIOCGMIIREG, &ifr) < 0) {
    fprintf(stderr, "mdio_read(): SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name,
        strerror(errno));
    return -1;
    }
    return mii->val_out;
}
#endif


#if HAS_ATH_ETHERNET_SWITCH_S17 || HAS_ATH_ETHERNET_SWITCH_S17_HWACCEL 
extern int _s17_getregvalue(const char *ifname, int port, char *reg_result);
extern int _s17_phy_link_up(const char *reg, const char *comp);
#endif

#if HAS_ATH_ETHERNET_SWITCH_S26
extern int _s26_getregvalue(const char *ifname, int port, char *reg_result);
extern int _s26_phy_link_up(const char *reg, const char *comp);
#endif

#if HAS_ATH_ETHERNET_SWITCH_S27 || HAS_ATH_ETHERNET_SWITCH_S27_HWACCEL 
extern int _s27_getregvalue(const char *ifname, int port, char *reg_result);
extern int _s27_phy_link_up(const char *reg, const char *comp);
#endif

/******************************************************/
/* This function checks the Link status of ifname,
    if it is linked, will return status=1; else return status=0;
    if there is error, the function will return FALSE.
*/
T_BOOL MiiCheckLink(const T_CHAR *ifname, T_INT *status)
{
#if HAS_ATH_ETHERNET_SWITCH_S17 || HAS_ATH_ETHERNET_SWITCH_S17_HWACCEL 
	char reg_result[32];
	int ret;

    if(0==strcmp("br0", ifname) || 0==strcmp("ath0", ifname) || 0==strcmp("br1", ifname))
    {
        *status = 1;
        return TRUE;
    }
#if WAN_AT_P1
    ret = _s17_getregvalue(ifname, 1, reg_result);//WAN:1
#else
    ret = _s17_getregvalue(ifname, 5, reg_result);//WAN:5
#endif
	if(ret)
    {
		ret = _s17_phy_link_up(reg_result, "0x0100");
    }

	*status = ret;

	return ret;	
#elif HAS_ATH_ETHERNET_SWITCH_S27 || HAS_ATH_ETHERNET_SWITCH_S27_HWACCEL 
	char reg_result[32];
	int ret;

    if(0==strcmp("br0", ifname) || 0==strcmp("ath0", ifname) || 0==strcmp("br1", ifname))
    {
        *status = 1;
        return TRUE;
    }

    ret = _s27_getregvalue(ifname, 5, reg_result);//WAN:5

	if(ret)
    {
		ret = _s27_phy_link_up(reg_result, "0x0100");
    }

	*status = ret;

	return ret;
#elif HAS_ATH_ETHERNET_SWITCH_S26
        char reg_result[32];
        int ret;

    if(0==strcmp("br0", ifname) || 0==strcmp("ath0", ifname) || 0==strcmp("br1", ifname))
    {
        *status = 1;
        return TRUE;
    }

    ret = _s26_getregvalue(ifname, 5, reg_result);//WAN:5

        if(ret)
    {
                ret = _s26_phy_link_up(reg_result, "0x0100");
    }

        *status = ret;

        return ret;	
#else

#define verbose (0) 
    T_INT32 res=TRUE;
    //char buf[100];
    int i, mii_val[32];
    //int bmcr, bmsr, advert, lkpar;
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;


    if(0==strcmp("br0", ifname) || 0==strcmp("ra0", ifname) || 0==strcmp("br1", ifname))
    {
        *status = 1;
        return TRUE;
    }

    mii->reg_num = MII_BMSR; /* read PHY Status */

    /* do check */
    if (ifname==0 || status==0)
    {
        printf("%s, ifname or status is NULL!\n", __FUNCTION__);
        return FALSE;
    }

    /* Open a basic socket. */
    if ((skfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) 
    {
        printf("%s: socket error!\n", __FUNCTION__);
        return FALSE;
    }

#if WAN_AT_P4
    mii->phy_id = 4;
#elif WAN_AT_P3
    mii->phy_id = 3;
#elif WAN_AT_P7
    mii->phy_id = 7;
#else
    mii->phy_id = 0;
#endif

    /* Get the vitals from the interface. */
    strncpy(ifr.ifr_name, WAN_IFNAME, IFNAMSIZ);
#if 0 /* marklin 20080218 : only read eth2.2(wan port) */
    if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0) 
    {
        if (errno != ENODEV)    
        {
            printf("SIOCGMIIPHY on '%s' failed: %s\n",
                    WAN_IFNAME, strerror(errno));
            res=FALSE;
            goto END_sysMiiIsLinked;
        }

    }


    mdio_read(skfd, MII_BMSR);
    for (i = 0; i < ((verbose > 1) ? 32 : 8); i++)
    {
        mii_val[i] = mdio_read(skfd, i);
    }

#if 0
    bmsr = mii_val[MII_BMSR];
    bmcr = mii_val[MII_BMCR]; 
    advert = mii_val[MII_ANAR]; 
    lkpar = mii_val[MII_ANLPAR];
#endif

END_sysMiiIsLinked:
#else

    mii->reg_num = 1; /* MII_BMSR */

    if (ioctl(skfd, SIOCGMIIREG, &ifr) < 0) {
        fprintf(stderr, "mdio_read(): SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name,strerror(errno));
        goto END_sysMiiIsLinked;
    }

    mii_val[1] = mii->val_out;

END_sysMiiIsLinked:
#endif
    close(skfd);
    *status=(mii_val[MII_BMSR] & MII_BMSR_LINK_VALID)?1:0;
    

 return res;
#endif /* HAS_ATH_ETHERNET_SWITCH_S17 || HAS_ATH_ETHERNET_SWITCH_S17_HWACCEL */
}
#else
static int mdio_read(int skfd, int location)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    mii->reg_num = location;
    if (ioctl(skfd, SIOCGMIIREG, &ifr) < 0) {
    fprintf(stderr, "mdio_read(): SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name,
        strerror(errno));
    return -1;
    }
    return mii->val_out;
}

/******************************************************/
/* This function checks the Link status of ifname,
    if it is linked, will return status=1; else return status=0;
    if there is error, the function will return FALSE.
*/
T_BOOL MiiCheckLink(const T_CHAR *ifname, T_INT *status)
{
#define verbose (0) 
    T_INT32 res=TRUE;
    char buf[100];
    int i, mii_val[32];
    //int bmcr, bmsr, advert, lkpar;
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;

    /* do check */
    if (ifname==0 || status==0)
    {
        printf("%s, ifname || status is NULL!\n", __FUNCTION__,ifname,status);
        return FALSE;
    }

    /* Open a basic socket. */
    if ((skfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) 
    {
        printf("%s: socket error!\n", __FUNCTION__);
        return FALSE;
    }

    /* Get the vitals from the interface. */
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0) 
    {
        if (errno != ENODEV)    
        {
            printf("SIOCGMIIPHY on '%s' failed: %s\n",
                    ifname, strerror(errno));
            res=FALSE;
            goto END_sysMiiIsLinked;
        }

    }


    mdio_read(skfd, MII_BMSR);
    for (i = 0; i < ((verbose > 1) ? 32 : 8); i++)
    {
        mii_val[i] = mdio_read(skfd, i);
    }

#if 0
    bmsr = mii_val[MII_BMSR];
    bmcr = mii_val[MII_BMCR]; 
    advert = mii_val[MII_ANAR]; 
    lkpar = mii_val[MII_ANLPAR];
#endif

END_sysMiiIsLinked:
    close(skfd);
    *status=(mii_val[MII_BMSR] & MII_BMSR_LINK_VALID)?1:0;
    

 return res;

}
#endif

#endif
/******************************************************/

