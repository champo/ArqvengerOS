#include "drivers/ext2/allocator.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"
#include "bitmap.h"

#define FIRST_USABLE_INODE 11

/**
 * Write the block bitmap for a group back to disk, from cache.
 *
 * @param fs The current fs.
 * @param group The group number.
 *
 * @return 0 on success, -1 on error
 */
static int write_block_bitmap(struct ext2* fs, int group);

/**
 * Read the block bitmap for a group from disk, to cache.
 *
 * @param fs The current fs.
 * @param group The group number.
 *
 * @return 0 on success, -1 on error
 */
static int read_block_bitmap(struct ext2* fs, int group);

/**
 * Write the inode bitmap for a group to disk, from cache.
 *
 * @param fs The current fs.
 * @param group The group number.
 *
 * @return 0 on success, -1 on error
 */
static int write_inode_bitmap(struct ext2* fs, int group);

/**
 * Read the inode bitmap for a group from disk, to cache.
 *
 * @param fs The current fs.
 * @param group The group number.
 *
 * @return 0 on success, -1 on error
 */
static int read_inode_bitmap(struct ext2* fs, int group);

/**
 * Try to allocate a block in a given block group.
 *
 * @param fs The current fs.
 * @param group The group number.
 *
 * @return 0 on error, the allocated block number otherwise
 */
static size_t allocate_in_group(struct ext2* fs, int blockGroup);

/**
 * Mark a logical block as used in its block group, and update related metadata.
 *
 * @param fs The current fs
 * @param blockGroup The block group the block belongs to.
 * @param block The block number in the group.
 *
 * @return 0 on success, -1 on error
 */
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

    // If the block size is 1024 bytes, then we have an empty block at the start
    size_t blockOffset = fs->sb->blockSize == 0;

    size_t bitmapLength = fs->sb->blocksPerBlockGroup;

    kprintf("Looking for block in group %u, with bitmap len %u, block start %u\n", blockGroup, bitmapLength,
            blockOffset + blockGroup * fs->sb->blocksPerBlockGroup);

    int block = bitmap_first_clear(fs->bitmapBuffer, bitmapLength);
    if (block == -1) {
        return 0;
    }

    reserve_block(fs, blockGroup, block);
    return blockOffset + blockGroup * fs->sb->blocksPerBlockGroup + block;
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

    // If the block size is 1024 bytes, then we have an empty block at the start
    size_t blockOffset = fs->sb->blockSize == 0;

    int blockGroup = (block - blockOffset) / fs->sb->blocksPerBlockGroup;
    int blockIndex = (block - blockOffset) % fs->sb->blocksPerBlockGroup;

    kprintf("Deallocating block %u, on group %u\n", block, blockGroup);

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

    size_t entries = fs->sb->inodesPerBlockGroup;
    for (size_t i = 0; i < fs->blockGroupCount; i++) {

        if (read_inode_bitmap(fs, i) == -1) {
            return 0;
        }

        int entry = bitmap_first_clear(fs->bitmapBuffer, entries);
        if (entry != -1 && entry + i * fs->sb->inodesPerBlockGroup >= FIRST_USABLE_INODE) {
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

    fs->sb->unallocatedInodes++;
    ext2_superblock_write(fs);

    fs->groupTable[blockGroup].unallocatedInodes++;
    ext2_write_blockgroup_table(fs);

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

