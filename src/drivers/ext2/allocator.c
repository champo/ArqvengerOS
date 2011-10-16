#include "drivers/ext2/allocator.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"
#include "bitmap.h"

static int write_block_bitmap(struct ext2* fs, int group);

static int read_block_bitmap(struct ext2* fs, int group);

static int write_inode_bitmap(struct ext2* fs, int group);

static int read_inode_bitmap(struct ext2* fs, int group);

static size_t allocate_in_group(struct ext2* fs, int blockGroup);

static int reserve_block(struct ext2* fs, int blockGroup, size_t block);

size_t allocate_block(struct ext2* fs, int groupHint) {

    size_t block;

    if ((block = allocate_in_group(fs, groupHint)) != 0) {
        return block;
    }

    for (size_t i = 0; i < fs->blockGroupCount; i++) {
        if ((block = allocate_in_group(fs, i)) != 0) {
            return block;
        }
    }

    return 0;
}

size_t allocate_in_group(struct ext2* fs, int blockGroup) {

    if (read_block_bitmap(fs, blockGroup) == -1) {
        return 0;
    }

    size_t bitmapLength = fs->sb->blocksPerBlockGroup / sizeof(unsigned char);
    int block = bitmap_first_clear(fs->bitmapBuffer, bitmapLength);
    if (block == -1) {
        return 0;
    }

    reserve_block(fs, blockGroup, block);
    return blockGroup * fs->sb->blocksPerBlockGroup + block;
}

int reserve_block(struct ext2* fs, int blockGroup, size_t block) {
    bitmap_set(fs->bitmapBuffer, block);
    write_block_bitmap(fs, blockGroup);

    fs->sb->unallocatedBlocks--;
    ext2_superblock_write(fs);

    fs->groupTable[blockGroup].unallocatedBlocks--;
    ext2_write_blockgroup_table(fs);

    return 0;
}

int write_block_bitmap(struct ext2* fs, int group) {
    if (fs->bitmapBlock != fs->groupTable[group].blockBitmapAddress) {
        return -1;
    }

    return write_block(fs, fs->bitmapBlock, fs->bitmapBuffer);
}

int read_block_bitmap(struct ext2* fs, int group) {

    if (fs->bitmapBlock == fs->groupTable[group].blockBitmapAddress) {
        return 0;
    }

    fs->bitmapBlock = fs->groupTable[group].blockBitmapAddress;
    return read_block(fs, fs->bitmapBlock, fs->bitmapBuffer);
}

int deallocate_block(struct ext2* fs, size_t block) {
    int blockGroup = block / fs->sb->blocksPerBlockGroup;
    int blockIndex = block % fs->sb->blocksPerBlockGroup;

    if (read_block_bitmap(fs, blockGroup) == -1) {
        return -1;
    }

    bitmap_clear(fs->bitmapBuffer, blockIndex);
    write_block_bitmap(fs, blockGroup);

    fs->sb->unallocatedBlocks++;
    ext2_superblock_write(fs);

    fs->groupTable[blockGroup].unallocatedBlocks++;
    ext2_write_blockgroup_table(fs);

    return 0;
}

size_t allocate_inode(struct ext2* fs) {

    size_t entries = fs->sb->inodesPerBlockGroup / sizeof(unsigned int);
    for (size_t i = 0; i < fs->blockGroupCount; i++) {

        if (read_inode_bitmap(fs, i) == -1) {
            return 0;
        }

        int entry = bitmap_first_clear(fs->bitmapBuffer, entries);
        if (entry != -1) {
            bitmap_set(fs->bitmapBuffer, entry);
            write_inode_bitmap(fs, i);

            fs->sb->unallocatedInodes--;
            ext2_superblock_write(fs);

            fs->groupTable[i].unallocatedInodes--;
            ext2_write_blockgroup_table(fs);

            return i * fs->sb->inodesPerBlockGroup + entry + 1;
        }
    }

    return 0;
}

int deallocate_inode(struct ext2* fs, size_t inode) {

    size_t blockGroup = inode / fs->sb->inodesPerBlockGroup;
    size_t index = (inode - 1) % fs->sb->inodesPerBlockGroup;

    if (read_inode_bitmap(fs, blockGroup) == -1) {
        return -1;
    }

    bitmap_clear(fs->bitmapBuffer, index);
    if (write_inode_bitmap(fs, blockGroup) == -1) {
        return -1;
    }

    return 0;
}

int write_inode_bitmap(struct ext2* fs, int group) {
    if (fs->bitmapBlock != fs->groupTable[group].inodeBitmapAddress) {
        return -1;
    }

    return write_block(fs, fs->bitmapBlock, fs->bitmapBuffer);
}

int read_inode_bitmap(struct ext2* fs, int group) {

    if (fs->bitmapBlock == fs->groupTable[group].inodeBitmapAddress) {
        return 0;
    }

    fs->bitmapBlock = fs->groupTable[group].inodeBitmapAddress;
    return read_block(fs, fs->bitmapBlock, fs->bitmapBuffer);
}

