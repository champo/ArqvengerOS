#ifndef __SUPERBLOCK__
#define __SUPERBLOCK__

//If extended fields needed, just ask!

int superblock_init(void);
int superblock_get_total_inodes(void);
int superblock_get_total_blocks(void);
int superblock_get_blocks_for_superuser(void);
int superblock_get_unallocated_blocks(void);
int superblock_get_unallocated_inodes(void);
int superblock_get_block_number(void);
int superblock_get_block_size(void);
int superblock_get_fragment_size(void);
int superblock_get_blocks_per_block_group(void);
int superblock_get_fragments_per_block_group(void);
int superblock_get_inodes_per_block_group(void);
int superblock_get_last_mount_time(void);
int superblock_get_last_written_time(void);
int superblock_get_total_mounts_since_last_check(void);
int superblock_get_total_mounts_before_next_check(void);
int supeblock_get_ext2_signature(void);
int superblock_get_file_system_state(void);
int superblock_get_error_action(void);
int superblock_get_version(void);
int superblock_get_last_consistency_check(void);
int superblock_get_interval_between_consistency_checks(void);
int superblock_get_OS_ID(void);
int superblock_get_user_ID_reserved_blocks(void);
int superblock_get_group_ID_reserved_blocks(void);
int superblock_end(void);
int superblock_get_total_block_groups(void);

#endif
