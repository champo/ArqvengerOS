#include "drivers/ata.h"
#include "system/mm.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"

#define BLOCK_GROUP_TABLE_START (SUPERBLOCK_START + SUPERBLOCK_SECTORS)

struct BlockGroupDescriptorTable* ext2_read_blockgroup_table(struct Superblock* sb) {
    size_t entries = ext2_get_total_block_groups(sb);
    size_t tableLen = sizeof(struct BlockGroupDescriptor) * entries;
    size_t sectors = (tableLen / SECTOR_SIZE) + 1;

    struct BlockGroupDescriptorTable* table = kalloc(
            sizeof(struct BlockGroupDescriptorTable) + sectors * SECTOR_SIZE
            );

    if (ata_read(BLOCK_GROUP_TABLE_START, sectors, table->descriptors) == -1) {
        return NULL;
    }

    table->length = entries;

    return table;
}

