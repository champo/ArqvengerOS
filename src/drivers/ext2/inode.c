#include "drivers/ext2/inode.h"
#include "system/mm.h"

struct ext2_Inode* ext2_read_inode(struct ext2* fs, size_t number) {

    if (fs->sb->totalInodes < number) {
        return NULL;
    }

    size_t blockGroup = number / fs->sb->blocksPerBlockGroup;
    size_t index = (number - 1) % fs->sb->blocksPerBlockGroup;

    if (blockGroup >= fs->groupTable->length) {
        return NULL;
    }

    size_t offsetInTable = sizeof(struct ext2_Inode) * index;
    size_t tableBlock = offsetInTable / fs->blockSize;

    struct ext2_Inode* inode = kalloc(sizeof(struct ext2_Inode));
    read_block_fragment(
        fs,
        fs->groupTable->descriptors[blockGroup].inodeTableStart + tableBlock,
        offsetInTable % fs->blockSize,
        sizeof(struct ext2_Inode),
        inode
    );

    return inode;
}

