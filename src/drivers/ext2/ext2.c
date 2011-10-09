#include "drivers/ext2/ext2.h"
#include "drivers/ext2/internal.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"
#include "system/mm.h"

struct ext2* ext2_load(unsigned long long startSector) {

    struct ext2* fs = kalloc(sizeof(struct ext2));

    fs->firstSector = startSector;
    ext2_superblock_init(fs);
    ext2_read_blockgroup_table(fs);
    fs->blockSize = 1024 << fs->sb->blockSize;
    fs->blockBuffer = kalloc(fs->blockSize);
    fs->sectorsPerBlock = fs->blockSize / SECTOR_SIZE;

    return fs;
}

