#ifndef __SN_PARTATION_TABLE_H__
#define __SN_PARTATION_TABLE_H__
//void sn_update_partition_tbl(struct mtd_info *ralink_mtd, uint64_t erasesize);
void sn_update_partition_tbl(struct mtd_info *mtd_info, uint32_t erasesize, struct mtd_partition_t phys_part_t[], int phys_part_size);
#endif
//__SN_PARTATION_TABLE_H__
