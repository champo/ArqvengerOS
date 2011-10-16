#include "drivers/ext2/allocator.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"

#define BYTE_FOR_BLOCK(block) fs->bitmapBuffer[(block) / sizeof(unsigned char)]
#define HAS_FREE_BLOCK(index) (fs->bitmapBuffer[(index)] != 0xFF)
#define BLOCK_TEST(block) ((BYTE_FOR_BLOCK(block) >> ((block) % sizeof(unsigned char))) & 0x1)
#define SET_BLOCK(block) (BYTE_FOR_BLOCK(block) |= (0x1 << ((block) % sizeof(unsigned char))))
#define UNSET_BLOCK(block) (BYTE_FOR_BLOCK(block) &= ~(1 << ((block) % sizeof(unsigned char))))

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
    for (size_t i = 0; i < bitmapLength; i++) {
        if (HAS_FREE_BLOCK(i)) {

            for (size_t j = 0; j < sizeof(unsigned char); j++) {
                if (!BLOCK_TEST(i * sizeof(unsigned char) + j)) {
                    reserve_block(fs, blockGroup, i * sizeof(unsigned char) + j);
                    if (!BLOCK_TEST(i * sizeof(unsigned char) + j)) kprintf("WTF");
                    return blockGroup * fs->sb->blocksPerBlockGroup + i * sizeof(unsigned char) + j;
                }
            }
        }
    }

    return 0;
}

int reserve_block(struct ext2* fs, int blockGroup, size_t block) {
    SET_BLOCK(block);
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

    UNSET_BLOCK(blockIndex);
    write_group_bitmap(fs, blockGroup);

    fs->sb->unallocatedBlocks++;
    ext2_superblock_write(fs);

    fs->groupTable[blockGroup].unallocatedBlocks++;
    ext2_write_blockgroup_table(fs);

    return 0;
}

