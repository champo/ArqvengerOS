#include "drivers/ata.h"
#include "system/mm.h"
#include "drivers/ext2/superblock.h"

#define SUPERBLOCK_SIZE 1024
#define SUPERBLOCK_SECTORS 2
#define SUPERBLOCK_START 2

struct Superblock* ext2_superblock_init(void) {
    struct Superblock* superblock;
    if ((superblock = kalloc(SUPERBLOCK_SIZE)) == NULL) {
        return NULL;
    }
    if (ata_read((unsigned long long)SUPERBLOCK_START, SUPERBLOCK_SECTORS, superblock) != 0) {
        return NULL;
    }
    return superblock;
}

int ext2_superblock_end(struct Superblock* superblock) {
    //TODO Free Superblock
    return 0;
}

int ext2_get_total_block_groups(struct Superblock* superblock) {
    return superblock->totalBlocks / superblock->blocksPerBlockGroup;
}

/*
char* superblock;

static int getBytes(int start, int end, char* buff);

int superblock_init(void) {
    if ((superblock = kalloc(1024)) == NULL) {
        return -1;
    }
    if (ata_read((unsigned long long)2, 2, superblock) != 0) {
        return -1;
    }
    return 0;
}

int superblock_get_total_inodes(void) {
    char buff[4];
    return getBytes(0, 3, buff);
}

int superblock_get_total_blocks(void) {
    char buff[4];
    return getBytes(4, 7, buff);
}

int superblock_get_blocks_for_superuser(void) {
    char buff[4];
    return getBytes(8, 11, buff);
}

int superblock_get_unallocated_blocks(void) {
    char buff[4];
    return getBytes(12, 15, buff);
}

int superblock_get_unallocated_inodes(void) {
    char buff[4];
    return getBytes(16, 19, buff);
}

int superblock_get_block_size(void) {
//TODO
    return 0;
}

int superblock_get_fragment_size(void) {
//TODO
    return 0;
}

int superblock_get_blocks_per_block_group(void) {
    char buff[4];
    return getBytes(32, 35, buff);
}

int superblock_get_fragments_per_block_group(void) {
    char buff[4];
    return getBytes(36, 39, buff);
}

int superblock_get_inodes_per_block_group(void) {
    char buff[4];
    return getBytes(40, 43, buff);
}

int superblock_get_last_mount_time(void) {
    char buff[4];
    return getBytes(44, 47, buff);
}

int superblock_get_last_written_time(void) {
    char buff[4];
    return getBytes(48, 51, buff);
}

int superblock_get_total_mounts_since_last_check(void) {
    char buff[2];
    return getBytes(52, 53, buff);
}

int superblock_get_total_mounts_before_next_check(void) {
    char buff[2];
    return getBytes(54, 55, buff);
}

int supeblock_get_ext2_signature(void) {
    char buff[2];
    return getBytes(56, 57, buff);
}

int superblock_get_file_system_state(void) {
    char buff[2];
    return getBytes(58, 59, buff);
}

int superblock_get_error_action(void) {
    char buff[2];
    return getBytes(60, 61, buff);
}

int superblock_get_version(void) {
    char buff[6];
    getBytes(76, 79, buff);
    getBytes(62, 63, buff + 4);
    return *((int*) buff);
}

int superblock_get_last_consistency_check(void) {
    char buff[4];
    return getBytes(64, 67, buff);
}

int superblock_get_interval_between_consistency_checks(void) {
    char buff[4];
    return getBytes(68, 71, buff);
}

int superblock_get_OS_ID(void) {
    char buff[4];
    return getBytes(72, 75, buff);
}

int superblock_get_user_ID_reserved_blocks(void) {
    char buff[2];
    return getBytes(80, 81, buff);
}

int superblock_get_group_ID_reserved_blocks(void) {
    char buff[2];
    return getBytes(82, 83, buff);
}

int superblock_get_block_number(void) {
    char buff[4];
    return getBytes(20, 23, buff);
}

int superblock_end(void) {
    //TODO Free Superblock
    return 0;
}

int superblock_get_total_block_groups(void) {
    return superblock_get_total_inodes() / superblock_get_inodes_per_block_group();
}

int getBytes(int start, int end, char* buff) {

    int i;

    for(i = 0; i < (end - start + 1); i++) {
        buff[i] = superblock[start + i];
    }
    return *((int*) buff);
}

*/
