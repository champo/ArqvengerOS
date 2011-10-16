#include "drivers/ext2/internal.h"

int read_sectors(struct ext2* fs, unsigned long long sector, size_t sectors, void* buffer) {
    return ata_read(fs->firstSector + sector, sectors, buffer);
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
            fs->fragmentReadBuffer = 0;
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
    return ata_write(fs->firstSector + sector, sectors, buffer);
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
            fs->fragmentReadBuffer = 0;
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


