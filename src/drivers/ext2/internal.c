#include "drivers/ext2/internal.h"
#include "system/cache/cache.h"
#include "debug.h"

int read_sectors(struct ext2* fs, unsigned long long sector, size_t sectors, void* buffer) {

    unsigned long long current = fs->firstSector + sector;
    unsigned long long end = current + sectors;

    char* to = (char*) buffer;
    char* from;

    // Ultra slow implementation for now
    for (size_t i = 0; current < end;) {
        struct CacheReference* ref = cache_get(current);
        assert(ref != NULL);

        mutex_lock(&ref->lock);

        size_t offset = current - ref->sector;
        size_t count = ref->count - offset;

        if (i + count > sectors) {
            count = sectors - i;
        }

        cache_access(ref);
        from = (char*) ref->page;
        assert(ref->page);
        memcpy(to + i * SECTOR_SIZE, from + offset * SECTOR_SIZE, count * SECTOR_SIZE);

        current += count;
        i += count;

        mutex_release(&ref->lock);
        cache_release(ref);
    }

    return 0;
}

int read_block(struct ext2* fs, size_t block, void* buffer) {

    if (block > fs->sb->totalBlocks) {
        return -1;
    }

    return read_sectors(fs, block * fs->sectorsPerBlock, fs->sectorsPerBlock, buffer);
}

int read_blocks(struct ext2* fs, size_t start, size_t blocks, void* buffer) {

    if (start + blocks > fs->sb->totalBlocks) {
        return -1;
    }

    return read_sectors(fs, start * fs->sectorsPerBlock, fs->sectorsPerBlock * blocks, buffer);
}

int read_block_fragment(struct ext2* fs, size_t block, size_t offset, size_t len, void* buffer) {

    if (fs->fragmentReadBlock != block) {
        fs->fragmentReadBlock = block;
        if (read_block(fs, block, fs->fragmentReadBuffer) == -1) {
            fs->fragmentReadBlock = 0;
            return -1;
        }
    }

    char* dest = buffer;
    char* origin = (char*) fs->fragmentReadBuffer + offset;

    for (size_t i = 0; i < len; i++) {
        dest[i] = origin[i];
    }

    return 0;
}

int write_sectors(struct ext2* fs, unsigned long long sector, size_t sectors, const void* buffer) {

    unsigned long long current = fs->firstSector + sector;
    unsigned long long end = current + sectors;

    const char* from = (char*) buffer;
    char* to;

    for (size_t i = 0; current < end;) {
        struct CacheReference* ref = cache_get(current);
        assert(ref);

        mutex_lock(&ref->lock);

        size_t offset = current - ref->sector;
        size_t count = ref->count - offset;

        if (i + count > sectors) {
            count = sectors - i;
        }

        cache_access(ref);
        to = (char*) ref->page;
        memcpy(to + offset * SECTOR_SIZE, from + i * SECTOR_SIZE, count * SECTOR_SIZE);

        current = ref->sector + ref->count;
        i += count;

        mutex_release(&ref->lock);
        cache_release(ref);
    }

    return 0;
}

int write_block(struct ext2* fs, size_t block, const void* buffer) {

    if (block > fs->sb->totalBlocks) {
        return -1;
    }

    return write_sectors(fs, block * fs->sectorsPerBlock, fs->sectorsPerBlock, buffer);
}

int write_blocks(struct ext2* fs, size_t start, size_t blocks, const void* buffer) {

    if (start + blocks > fs->sb->totalBlocks) {
        return -1;
    }

    return write_sectors(fs, start * fs->sectorsPerBlock, blocks * fs->sectorsPerBlock, buffer);
}

int write_block_fragment(struct ext2* fs, size_t block, size_t offset, size_t len, const void* buffer) {

    if (fs->fragmentReadBlock != block) {
        fs->fragmentReadBlock = block;
        if (read_block(fs, block, fs->fragmentReadBuffer) == -1) {
            fs->fragmentReadBlock = 0;
            return -1;
        }
    }

    char* dest = (char*) fs->fragmentReadBuffer + offset;
    const char* origin = buffer;

    for (size_t i = 0; i < len; i++) {
        dest[i] = origin[i];
    }

    return write_block(fs, block, fs->fragmentReadBuffer);
}


