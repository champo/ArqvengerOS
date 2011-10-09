#include "drivers/ext2/ext2.h"
#include "drivers/ext2/internal.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"
#include "system/mm.h"
#include "system/kprintf.h"

struct ext2* ext2_load(unsigned long long startSector) {

    struct ext2* fs = kalloc(sizeof(struct ext2));

    fs->firstSector = startSector;

    ext2_superblock_init(fs);

    fs->blockSize = 1024 << fs->sb->blockSize;
    fs->sectorsPerBlock = fs->blockSize / SECTOR_SIZE;
    fs->blockGroupCount = ext2_get_total_block_groups(fs->sb);

    fs->blockBuffer = kalloc(fs->blockSize);
    for (int i = 0; i < 3; i++) {
        fs->blockIndexAddress[i] = 0;
        fs->blockIndexBuffer[i] = kalloc(fs->blockSize);
    }

    ext2_read_blockgroup_table(fs);

    kprintf("Block size %u Sectors per block %u\n", fs->blockSize, fs->sectorsPerBlock);
    for (size_t i = 0; i < fs->blockGroupCount; i++) {
        kprintf("Block Group %d: free blocks %u, table start %u\n", i, fs->groupTable[i].unallocatedBlocks, fs->groupTable[i].inodeTableStart);
    }


    return fs;
}

