#define DO_PRINTK 1
/*
 * Update the MTD partition table.
 *
 * Author:  Senao Networks Inc.
 *
 * Copyright (c) 2010, Senao Networks Inc.
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
 
//#include "../maps/ralink-flash.h"

#include "sn_flash_map.h"
#include "sn_partition_table.h"
#include "gconfig.h"
 

#if DYNAMIC_UDPATE_PARTITION_TBL
/* Firmware image header, copy from bootloader hdr definition(boot/include/image.h) */
#define IH_NMLEN                32      /* Image Name Length            */
typedef struct image_header {
        uint32_t        ih_magic;       /* Image Header Magic Number    */
        uint32_t        ih_hcrc;        /* Image Header CRC Checksum    */
        uint32_t        ih_time;        /* Image Creation Timestamp     */
        uint32_t        ih_size;        /* Image Data Size              */
        uint32_t        ih_load;        /* Data  Load  Address          */
        uint32_t        ih_ep;          /* Entry Point Address          */
        uint32_t        ih_dcrc;        /* Image Data CRC Checksum      */
        uint8_t         ih_os;          /* Operating System             */
        uint8_t         ih_arch;        /* CPU architecture             */
        uint8_t         ih_type;        /* Image Type                   */
        uint8_t         ih_comp;        /* Compression Type             */
        uint8_t         ih_name[IH_NMLEN];      /* Image Name           */
} image_header_t;
#endif





#if DYNAMIC_UDPATE_PARTITION_TBL
void sn_update_partition_tbl(struct mtd_info *mtd_info, uint32_t erasesize, struct mtd_partition_t phys_part_t[], int phys_part_size)
{
        int i;
        int knl_idx = 0;
        int cb_app_idx = 0;
        int retlen;
        uint64_t kernel_size, kernel_pading_size;
        uint32_t rem;
        char headerBuf[sizeof(image_header_t)];

        /*Find kernel/cb-app index first*/
        for (i=0; i<phys_part_size; i++)
        {
#if COMBINED_FW_IMAGE_TYPE == 2
                if (!strncmp(phys_part_t[i].name, APP_STORAGE_PARTITION_NAME, sizeof(APP_STORAGE_PARTITION_NAME)))
#else
                if (!strncmp(phys_part_t[i].name, KERNEL_PARTITION_NAME, sizeof(KERNEL_PARTITION_NAME)))
#endif
                {
                        knl_idx = i;
                }
    
#if COMBINED_FW_IMAGE_TYPE == 2
                if (!strncmp(phys_part_t[i].name, COMBINED_APP_PARTITION_NAME, sizeof(COMBINED_APP_PARTITION_NAME)))
#else
                if (!strncmp(phys_part_t[i].name, APP_STORAGE_PARTITION_NAME, sizeof(APP_STORAGE_PARTITION_NAME)))
#endif
                {
                        cb_app_idx = i;
                }
        }

        // protect
        if(knl_idx == 0) return;
        if(cb_app_idx == 0) return;

        mtd_info->read(mtd_info, (unsigned int)(phys_part_t[knl_idx].offset), sizeof(image_header_t), &retlen, headerBuf);

        // get kernel size without app
        kernel_size = ((headerBuf[12] & 0xff) << 24) + ((headerBuf[13] & 0xff) << 16) + ((headerBuf[14] & 0xff) << 8) + ((headerBuf[15] & 0xff) << 0);

		// kernel real size
        // 00000000  27 05 19 56 6c cf 84 2b  4e 4d da 8f 00 13 5f fb  |'..Vl..+NM...._.|
		// 00000010  80 00 00 00 80 2c e0 00  1d b9 ba 90 05 05 02 03  |.....,..........|
		// 00000020  4c 69 6e 75 78 20 4b 65  72 6e 65 6c 20 49 6d 61  |Linux Kernel Ima|
		// 00000030  67 65 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |ge..............|
		// 00000040  5d 00 00 00 02 35 65 33  00 00 00 00 00 00 00 6f  |]....5e3.......o|
		// 0x40 is header size.
		kernel_size += 0x40;

        // cal kernel pading size
       // kernel_pading_size = kernel_size + erasesize-(kernel_size%erasesize);
        div_u64_rem(kernel_size, erasesize, &rem);
        kernel_pading_size = kernel_size + ((rem==0)?0:erasesize-rem);
#if 1  
        /*New offset of app = kernel offset + real kernel size*/
        /*New size of app = logical kernel size - real kernel size*/
        phys_part_t[cb_app_idx].offset = phys_part_t[knl_idx].offset + kernel_pading_size;
      
        phys_part_t[cb_app_idx].size = phys_part_t[knl_idx].size - kernel_pading_size;
#endif
}
#endif

