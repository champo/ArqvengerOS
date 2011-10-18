#include "drivers/ext2/ext2.h"
#include "drivers/ext2/internal.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"
#include "system/mm.h"

struct ext2* ext2_load(unsigned long long startSector) {

    struct ext2* fs = kalloc(sizeof(struct ext2));

    fs->firstSector = startSector;

    ext2_superblock_init(fs);

    fs->blockSize = 1024 << fs->sb->blockSize;
    fs->sectorsPerBlock = fs->blockSize / SECTOR_SIZE;
    fs->blockGroupCount = ext2_get_total_block_groups(fs->sb);

    // Setup the various caches

    for (int i = 0; i < 3; i++) {
        fs->blockIndexAddress[i] = 0;
        fs->blockIndexBuffer[i] = kalloc(fs->blockSize);
    }

    fs->fragmentReadBlock = 0;
    fs->fragmentReadBuffer = kalloc(fs->blockSize);

    fs->evictBlockBuffer = 0;
    for (int i = 0; i < BLOCK_BUFFER_COUNT; i++) {
        fs->blockBufferAddress[i] = 0;
        fs->blockBufferOwner[i] = NULL;
        fs->blockBuffer[i] = kalloc(fs->blockSize);
    }

    fs->bitmapBlock = 0;
    fs->bitmapBuffer = kalloc(fs->blockSize);

    ext2_read_blockgroup_table(fs);

    return fs;
}

