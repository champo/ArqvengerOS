#include "drivers/ext2/blockGroup.h"
#include "system/mm.h"

#define BLOCK_GROUP_TABLE_START \
    ((unsigned long long)(SUPERBLOCK_START + SUPERBLOCK_SECTORS))

int ext2_read_blockgroup_table(struct ext2* fs) {
    size_t entries = ext2_get_total_block_groups(fs->sb);
    size_t tableLen = sizeof(struct BlockGroupDescriptor) * entries;
    size_t sectors = (tableLen / SECTOR_SIZE) + 1;

    struct BlockGroupDescriptorTable* table = kalloc(
            sizeof(struct BlockGroupDescriptorTable) + sectors * SECTOR_SIZE
            );

    if (read_sectors(fs, BLOCK_GROUP_TABLE_START, sectors, table->descriptors) == -1) {
        return -1;
    }

    table->length = entries;

    fs->groupTable = table;

    return 0;
}

