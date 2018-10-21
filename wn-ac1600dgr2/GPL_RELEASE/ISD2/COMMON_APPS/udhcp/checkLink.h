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

#ifndef _CHECKLINK_H_
#define _CHECKLINK_H_
#include <stdio.h>
#include <stdlib.h>
//#include <netinet/in.h>
#include <sap_ostypes.h>
//#include <netinet/in.h>
#include <net/if.h>
//#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h> /* out of habit, I use a SOCK_PACKET socket*/
 

#ifdef __cplusplus
extern "C" {
#endif 

T_BOOL MiiCheckLink(const T_CHAR *ifname, T_INT *status);
#ifdef __cplusplus
}
#endif 

#endif


