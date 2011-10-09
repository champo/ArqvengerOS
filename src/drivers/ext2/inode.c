#include "drivers/ext2/inode.h"
#include "system/mm.h"

static int read_inode_block(struct ext2* fs, struct ext2_Inode* inode, size_t blockIndex, void* buffer, size_t offset, size_t len);

static int read_block_index(struct ext2* fs, int level, size_t block);

struct ext2_Inode* ext2_read_inode(struct ext2* fs, size_t number) {

    if (fs->sb->totalInodes < number) {
        return NULL;
    }

    size_t blockGroup = number / fs->sb->blocksPerBlockGroup;
    size_t index = (number - 1) % fs->sb->blocksPerBlockGroup;

    if (blockGroup >= fs->blockGroupCount) {
        return NULL;
    }

    size_t offsetInTable = sizeof(struct ext2_Inode) * index;
    size_t tableBlock = offsetInTable / fs->blockSize;

    struct ext2_Inode* inode = kalloc(sizeof(struct ext2_Inode));
    read_block_fragment(
        fs,
        fs->groupTable[blockGroup].inodeTableStart + tableBlock,
        offsetInTable % fs->blockSize,
        sizeof(struct ext2_Inode),
        inode
    );

    return inode;
}

int ext2_read_inode_content(struct ext2* fs, struct ext2_Inode* inode, size_t offset, size_t size, void* buffer) {

    if (offset > inode->size) {
        return -1;
    } else if (offset + size > inode->size) {
        size = inode->size - offset;
    }

    size_t firstBlockIndex = offset / fs->blockSize;
    size_t endBlockIndex = (offset + size) / fs->blockSize;

    char* buf = buffer;
    for (size_t block = firstBlockIndex; block < endBlockIndex; block++) {

        if (block == firstBlockIndex) {

            size_t offsetInBlock = offset % fs->blockSize;
            read_inode_block(
                fs,
                inode,
                block,
                buf,
                offsetInBlock,
                fs->blockSize - offsetInBlock
            );

            buf += fs->blockSize - offsetInBlock;

        } else if (block == endBlockIndex) {

            read_inode_block(
                fs,
                inode,
                block,
                buf,
                0,
                (offset + size) % fs->blockSize
            );

        } else {
            read_inode_block(fs, inode, block, buffer, 0, fs->blockSize);
            buf += fs->blockSize;
        }
    }

    return 0;
}

int read_inode_block(struct ext2* fs, struct ext2_Inode* inode, size_t blockIndex, void* buffer, size_t offset, size_t len) {

    size_t block;
    size_t pointersPerBlock = fs->blockSize / sizeof(size_t);

    if (blockIndex < 12) {
        block = inode->directBlockPointers[blockIndex];
    } else if (blockIndex - 12 < pointersPerBlock) {

        read_block_index(fs, 0, inode->singlyIndirectBlockPointer);
        block = fs->blockIndexBuffer[0][blockIndex - 12];

    } else if (blockIndex - 12 - pointersPerBlock < pointersPerBlock * pointersPerBlock) {

        size_t offsetInLevel = blockIndex - 12 - pointersPerBlock;
        size_t firstLevel = offsetInLevel / pointersPerBlock;
        size_t secondLevel = offsetInLevel % pointersPerBlock;

        read_block_index(fs, 0, inode->doublyIndirectBlockPointer);
        read_block_index(fs, 1, fs->blockIndexBuffer[0][firstLevel]);

        block = fs->blockIndexBuffer[1][secondLevel];

    } else {

        size_t pointersInSecondLevel = pointersPerBlock * pointersPerBlock;
        size_t offsetInLevel = blockIndex - 12 - pointersPerBlock - pointersInSecondLevel;

        size_t firstLevel = offsetInLevel / pointersInSecondLevel;
        size_t secondLevel = (offsetInLevel / pointersPerBlock) % pointersPerBlock;
        size_t thirdLevel = offsetInLevel % pointersPerBlock;

        read_block_index(fs, 0, inode->triplyIndirectBlockPointer);
        read_block_index(fs, 1, fs->blockIndexBuffer[0][firstLevel]);
        read_block_index(fs, 2, fs->blockIndexBuffer[1][secondLevel]);

        block = fs->blockIndexBuffer[2][thirdLevel];
    }

    if (offset == 0 && len == fs->blockSize) {
        return read_block(fs, block, buffer);
    } else {
        return read_block_fragment(fs, block, offset, len, buffer);
    }
}

int read_block_index(struct ext2* fs, int level, size_t block) {

    if (fs->blockIndexAddress[level] != block) {
        fs->blockIndexAddress[level] = block;
        return read_block(fs, fs->blockIndexAddress[level], fs->blockIndexBuffer[level]);
    }

    return 0;
}

