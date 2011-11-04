#include "drivers/ext2/blockGroup.h"
#include "drivers/ext2/superblock.h"
#include "system/mm/allocator.h"
#include "system/kprintf.h"

#define SUPERBLOCK_END \
    ((unsigned long long)(SUPERBLOCK_START + SUPERBLOCK_SECTORS))

int ext2_read_blockgroup_table(struct ext2* fs) {

    size_t block;
    // The superblock is always in the same place, and the table is always the next block
    // So we need to take into account the block size to know which block that is
    if (SUPERBLOCK_END <= fs->sectorsPerBlock) {
        block = 1;
    } else {
        block = 2;
    }

    size_t tableSize = fs->blockGroupCount * sizeof(struct BlockGroupDescriptor);
    size_t blocks = (tableSize / fs->blockSize) + 1;
    struct BlockGroupDescriptor* table = kalloc(fs->blockSize * blocks);

    if (read_blocks(fs, block, blocks, table) == -1) {
        return -1;
    }

    fs->groupTable = table;

    return 0;
}

int ext2_write_blockgroup_table(struct ext2* fs) {

    size_t block;
    if (SUPERBLOCK_END <= fs->sectorsPerBlock) {
        block = 1;
    } else {
        block = 2;
    }

    size_t tableSize = fs->blockGroupCount * sizeof(struct BlockGroupDescriptor);
    size_t blocks = (tableSize / fs->blockSize) + 1;

    return write_blocks(fs, block, blocks, fs->groupTable);
}

