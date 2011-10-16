#include "drivers/ext2/blockGroup.h"
#include "drivers/ext2/superblock.h"
#include "system/mm.h"
#include "system/kprintf.h"

#define SUPERBLOCK_END \
    ((unsigned long long)(SUPERBLOCK_START + SUPERBLOCK_SECTORS))

int ext2_read_blockgroup_table(struct ext2* fs) {

    size_t block;
    if (SUPERBLOCK_END <= fs->sectorsPerBlock) {
        block = 1;
    } else {
        block = 2;
    }

    struct BlockGroupDescriptor* table = kalloc(fs->blockSize);

    if (read_block(fs, block, table) == -1) {
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


    return write_block(fs, block, fs->groupTable);
}

