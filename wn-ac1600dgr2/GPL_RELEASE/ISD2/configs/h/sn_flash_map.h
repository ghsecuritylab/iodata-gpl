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
;    Project : WN-AC1600DGR_SCO
;    Creator :
;    File    : sn_flash_map.h
;    Abstract: Flash partition related
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;       leonard         2013/1/23       Newly Create
;*****************************************************************************/

#ifndef _SN_FLASH_MAP_H_
#define _SN_FLASH_MAP_H_

//#include <linux/types.h>
//#include <linux/mtd/partitions.h>
#ifndef MTDPART_SIZ_FULL
#define MTDPART_SIZ_FULL        (0)
#endif

/*-------------------------------------------------------------------------*/
/*                        RTL8196C SOC Flash map                           */
/*-------------------------------------------------------------------------*/
#define USER_DEFINITION_FLASH_MAP
#define DYNAMIC_UDPATE_PARTITION_TBL    1   /*dynamic update cb-app partition table offset*/
/* combine firmware type:
   1. Kernel | app
   2. app    | cb-app */
#define COMBINED_FW_IMAGE_TYPE      1 

#define MTD_WRITEABLE				0x400

#if 1 //we should try to include the defination of kernel but not define here.
// the linux 2.6.31 changed the mtd_partition to support 64bit variables!

struct mtd_partition_t {
        char *name;                     /* identifier string */
        unsigned long long size;                  /* partition size */
        unsigned long long offset;                /* offset within the master MTD space */
        unsigned int mask_flags;            /* master MTD flags to mask out for this partition */
        unsigned int reserved1;  /* pointer address for struct nand_ecclayout *ecclayout */
};
#else
struct mtd_partition_t {
        char *name;              /* identifier string */
        unsigned int size;       /* partition size */
        unsigned int offset;     /* offset within the master MTD space */
        unsigned int mask_flags; /* master MTD flags to mask out for this partition */
        unsigned int reserved1;  /* pointer address for struct nand_ecclayout *ecclayout */
};
#endif

static struct mtd_partition_t physmap_partitions_t[] = {
        {
                .name        =   "ALL",
                .size        =   MTDPART_SIZ_FULL,
                .offset      =   0,
        },
        /* Put your own partition definitions here */
        {
                .name        =   "Bootloader", /* mtdblock0, U-boot */
                .size        =   0x30000, /* 192KB, include revovery WEB, fake DNS proxy and DHCP server */
                .offset      =   0,
        }, {
                .name        =   "Config", /* mtdblock1, U-Boot configuration, including device MAC, HWID, SN, OP_MODE... */
                .size        =   0x10000,       
                .offset      =   0x30000,
        }, { 
                .name        =   "Kernel", /* mtdblock2 : Kernel + (busybox + iptables..) */
                .size        =   0xE50000, /* AllInOne image is including Kernel+rootfs */
                .offset      =   0x40000,
        }, {
                .name        =   "apps", /* Appliactions */    
                .size        =   0xCC0000,
                .offset      =   0x1D0000,
        }, {
                .name        =   "manufacture", /* manufacture tools/image */
                .size        =   0x100000, /* 1MB */
                .offset      =   0xE90000,
        }, { 
                .name        =   "backup", /* backup partition for Config and caldata */ 
                .size        =   0x10000, /* 64 KB */
                .offset      =   0xF90000,
        }, { 
                .name        =   "storage", /* configuartion: jffs2 at less 5 sectors */ 
                .size        =   0x50000,       
                .offset      =   0xFA0000,
        }, { 
                .name        =   "caldata", /* calibration data */ 
                .size        =   0x10000,       
                .offset      =   0xFF0000,
        } 
};
#endif /*SN_FLASH_MAP_T*/
