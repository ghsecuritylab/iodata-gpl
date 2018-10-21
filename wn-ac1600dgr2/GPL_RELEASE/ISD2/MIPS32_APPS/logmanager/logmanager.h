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
;    Project : ISD
;    Creator : John Chang
;    File    : logmanager.h
;    Abstract: Define the log manager constants and variables
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;
;*****************************************************************************/
#ifndef _LOG_MANAGER_H
#define _LOG_MANAGER_H


#define LOG_SOCK_PATH           "/tmp/log_socket"
#define LOG_CONFIG_FILE         "/etc/logserver.conf"
#define LOG_TMP_FILE            "/tmp/log_tmp"
#if HAS_PARENTAL_CONTROL
#define LOG_TMP_FILE_PC            "/tmp/log_tmp_pc"
#endif
#define LOG_CONFIG_LINE_LEN     100
#define LOG_EMAIL_LEN           30
#if HAS_PARENTAL_CONTROL
#define LOG_MAXLINE         	1024    /* maximum line length */
#else
#define LOG_MAXLINE         	512    /* maximum line length */
#endif
#define LOG_EMAIL_SUBJECT_LEN	32

/* Log service type */
#define LOG_SERVICE_MASK_SYSLOG  (1<<0)
#define LOG_SERVICE_MASK_EMAIL   (1<<1)
#define LOG_SERVICE_MASK_SNMP    (1<<2)
#define LOG_SERVICE_MASK_FILE    (1<<3)
#define LOG_SERVICE_MASK_POOL    (1<<4)
#define LOG_SERVICE_MASK_NETWORK (1<<5)
#if HAS_PARENTAL_CONTROL
#define LOG_SERVICE_MASK_FILE_PC	(1<<6)
#endif


/* The logger */
enum LOG_LOGGER
{
    LOG_LOGGER_SERVER = 0,
    LOG_LOGGER_SYSTEM,
    LOG_LOGGER_SNMP,
    LOG_LOGGER_ACL,
    LOG_LOGGER_DHCP,
	LOG_LOGGER_DEBUG,
	LOG_LOGGER_ATTACK,
	LOG_LOGGER_DROP,
	LOG_LOGGER_NOTICE,
    LOG_LOGGER_VOIP,
#if HAS_PARENTAL_CONTROL
	LOG_LOGGER_PARENTAL_CONTROL,
#endif
    /* count the number */
    LOG_LOGGER_COUNT
};

/* Logger Mask */
#define LOG_LOGGER_MASK_SERVER (1<<LOG_LOGGER_SERVER)
#define LOG_LOGGER_MASK_SYSTEM (1<<LOG_LOGGER_SYSTEM)
#define LOG_LOGGER_MASK_SNMP   (1<<LOG_LOGGER_SNMP)
#define LOG_LOGGER_MASK_ACL    (1<<LOG_LOGGER_ACL)
#define LOG_LOGGER_MASK_DHCP   (1<<LOG_LOGGER_DHCP)
#define LOG_LOGGER_MASK_DEBUG  (1<<LOG_LOGGER_DEBUG)
#define LOG_LOGGER_MASK_ATTACK (1<<LOG_LOGGER_ATTACK)
#define LOG_LOGGER_MASK_DROP   (1<<LOG_LOGGER_DROP)
#define LOG_LOGGER_MASK_NOTICE (1<<LOG_LOGGER_NOTICE)
#define LOG_LOGGER_MASK_VOIP   (1<<LOG_LOGGER_VOIP)
#if HAS_PARENTAL_CONTROL
#define LOG_LOGGER_MASK_PARENTAL_CONTROL   (1<<LOG_LOGGER_PARENTAL_CONTROL)
#endif

struct logger_config
{
    int  id;
    int  mask;
    const char *name;
    const char *daemon;
};

static struct logger_config logger_info[]=
{
    { LOG_LOGGER_SERVER, LOG_LOGGER_MASK_SERVER,  "[LOGSVR]",   "logsvrd" },
    { LOG_LOGGER_SYSTEM, LOG_LOGGER_MASK_SYSTEM,  "[SYSTEM]",   "systemd" },
    { LOG_LOGGER_SNMP,   LOG_LOGGER_MASK_SNMP,    "[SNMP]",     "snmpmsg" },
    { LOG_LOGGER_ACL,    LOG_LOGGER_MASK_ACL,     "[ACL]",      "acld"    },
    { LOG_LOGGER_DHCP,   LOG_LOGGER_MASK_DHCP,    "[DHCP]",     "dhcpd"   },
	{ LOG_LOGGER_DEBUG,  LOG_LOGGER_MASK_DEBUG,   "[DEBUG]",    "debugd"  },
	{ LOG_LOGGER_ATTACK, LOG_LOGGER_MASK_ATTACK,  "[ATTACK]",   "attackd" },
	{ LOG_LOGGER_DROP,   LOG_LOGGER_MASK_DROP,    "[DROP]",     "dropd"   },
	{ LOG_LOGGER_NOTICE, LOG_LOGGER_MASK_NOTICE,  "[NOTICE]",   "noticed" },
    { LOG_LOGGER_VOIP,   LOG_LOGGER_MASK_VOIP,    "[VoIP]",     "voipd"   }
#if HAS_PARENTAL_CONTROL
    ,{ LOG_LOGGER_PARENTAL_CONTROL,   LOG_LOGGER_MASK_PARENTAL_CONTROL,    "[Parental_Control]",     "pcd"   }
#endif

};


struct log_element
{
    int  logger;
    int  len;
    unsigned char data[LOG_MAXLINE + 1];
};

#endif /* _LOG_MANAGER_H */
