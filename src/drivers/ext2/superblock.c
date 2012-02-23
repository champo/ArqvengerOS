#include "drivers/ext2/superblock.h"
#include "system/mm/allocator.h"
#include "system/cache/cache.h"
#include "debug.h"

int ext2_superblock_init(struct ext2* fs) {

    struct CacheReference* ref = cache_get(SUPERBLOCK_START);
#if SUPERBLOCK_START + SUPERBLOCK_SECTOR > SECTORS_PER_PAGE
#error
#endif

    assert(ref);
    assert(ref->page);

    cache_freeze(ref);
    fs->sb = (char*) ref->page + SECTOR_SIZE * SUPERBLOCK_START;

    return 0;
}

int ext2_superblock_end(struct ext2* fs) {
    cache_sync(1);
    return 0;
}

int ext2_get_total_block_groups(struct Superblock* superblock) {
    return (superblock->totalBlocks / superblock->blocksPerBlockGroup) + 1;
}

int ext2_superblock_write(struct ext2* fs) {
    return 0;
}

