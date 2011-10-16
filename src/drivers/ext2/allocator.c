#include "drivers/ext2/allocator.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"
#include "bitmap.h"

static int write_group_bitmap(struct ext2* fs, int group);

static int load_group_bitmap(struct ext2* fs, int group);

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

    if (load_group_bitmap(fs, blockGroup) == -1) {
        return 0;
    }

    size_t bitmapLength = fs->sb->blocksPerBlockGroup / sizeof(unsigned char);
    size_t block = bitmap_first_clear(fs->bitmapBuffer, bitmapLength);
    if (block == -1) {
        return 0;
    }

    reserve_block(fs, blockGroup, block);
    return blockGroup * fs->sb->blocksPerBlockGroup + block;
}

int reserve_block(struct ext2* fs, int blockGroup, size_t block) {
    bitmap_set(fs->bitmapBuffer, block);
    write_group_bitmap(fs, blockGroup);

    fs->sb->unallocatedBlocks--;
    ext2_superblock_write(fs);

    fs->groupTable[blockGroup].unallocatedBlocks--;
    ext2_write_blockgroup_table(fs);

    return 0;
}

int write_group_bitmap(struct ext2* fs, int group) {
    if (fs->bitmapBlock != fs->groupTable[group].blockBitmapAddress) {
        return -1;
    }

    return write_block(fs, fs->bitmapBlock, fs->bitmapBuffer);
}

int load_group_bitmap(struct ext2* fs, int group) {

    if (fs->bitmapBlock == fs->groupTable[group].blockBitmapAddress) {
        return 0;
    }

    fs->bitmapBlock = fs->groupTable[group].blockBitmapAddress;
    return read_block(fs, fs->bitmapBlock, fs->bitmapBuffer);
}

int deallocate_block(struct ext2* fs, size_t block) {
    int blockGroup = block / fs->sb->blocksPerBlockGroup;
    int blockIndex = block % fs->sb->blocksPerBlockGroup;

    if (load_group_bitmap(fs, blockGroup) == -1) {
        return -1;
    }

    bitmap_clear(fs->bitmapBuffer, blockIndex);
    write_group_bitmap(fs, blockGroup);

    fs->sb->unallocatedBlocks++;
    ext2_superblock_write(fs);

    fs->groupTable[blockGroup].unallocatedBlocks++;
    ext2_write_blockgroup_table(fs);

    return 0;
}

