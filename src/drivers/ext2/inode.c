#include "drivers/ext2/inode.h"
#include "system/mm.h"
#include "system/fs/inode.h"

static int read_inode_block(struct fs_Inode* inode, size_t blockIndex, void* buffer, size_t offset, size_t len);

static int read_block_index(struct ext2* fs, int level, size_t block);

static size_t block_index_to_block(struct ext2* fs, struct ext2_Inode* inode, size_t blockIndex);

struct fs_Inode* ext2_read_inode(struct ext2* fs, size_t number) {

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

    struct fs_Inode* fs_inode = kalloc(sizeof(struct fs_Inode));
    fs_inode->data = kalloc(sizeof(struct ext2_Inode));
    fs_inode->fileSystem = fs;
    fs_inode->number = number;
    read_block_fragment(
        fs,
        fs->groupTable[blockGroup].inodeTableStart + tableBlock,
        offsetInTable % fs->blockSize,
        sizeof(struct ext2_Inode),
        fs_inode->data
    );

    return fs_inode;
}

int ext2_read_inode_content(struct fs_Inode* inode, size_t offset, size_t size, void* buffer) {

    if (offset > inode->data->size) {
        return -1;
    } else if (offset + size > inode->data->size) {
        size = inode->data->size - offset;
    }

    size_t firstBlockIndex = offset / inode->fileSystem->blockSize;
    size_t endBlockIndex = (offset + size) / inode->fileSystem->blockSize;

    char* buf = buffer;
    for (size_t block = firstBlockIndex; block <= endBlockIndex; block++) {

        if (block == firstBlockIndex) {

            size_t offsetInBlock = offset % inode->fileSystem->blockSize;
            size_t partSize = inode->fileSystem->blockSize - offsetInBlock;
            if (size < partSize) {
                partSize = size;
            }

            read_inode_block(
                inode,
                block,
                buf,
                offsetInBlock,
                partSize
            );

            buf += inode->fileSystem->blockSize - offsetInBlock;

        } else if (block == endBlockIndex) {

            read_inode_block(
                inode,
                block,
                buf,
                0,
                (offset + size) % inode->fileSystem->blockSize
            );

        } else {
            read_inode_block(inode, block, buffer, 0, inode->fileSystem->blockSize);
            buf += inode->fileSystem->blockSize;
        }
    }

    return 0;
}

static int read_inode_block(struct fs_Inode* inode, size_t blockIndex, void* buffer, size_t offset, size_t len) {

    size_t block = block_index_to_block(inode->fileSystem, inode->data, blockIndex);

    size_t bufferIndex;
    for (bufferIndex = 0 ; bufferIndex < BLOCK_BUFFER_COUNT; bufferIndex++) {
        if (inode->fileSystem->blockBufferOwner[bufferIndex] == inode->data) {
            break;
        }
    }

    if (bufferIndex == BLOCK_BUFFER_COUNT) {
        for (bufferIndex = 0; bufferIndex < BLOCK_BUFFER_COUNT; bufferIndex++) {
            if (inode->fileSystem->blockBufferOwner[bufferIndex] == NULL) {
                break;
            }
        }

        if (bufferIndex == BLOCK_BUFFER_COUNT) {
            bufferIndex = inode->fileSystem->evictBlockBuffer++;
        }

        inode->fileSystem->blockBufferOwner[bufferIndex] = inode->data;
        inode->fileSystem->blockBufferAddress[bufferIndex] = 0;
    }

    if (inode->fileSystem->blockBufferAddress[bufferIndex] != block) {

        inode->fileSystem->blockBufferAddress[bufferIndex] = block;
        if (read_block(inode->fileSystem, block, inode->fileSystem->blockBuffer[bufferIndex]) == -1) {
            return -1;
        }
    }

    char* from = (char*) inode->fileSystem->blockBuffer[bufferIndex] + offset;
    char* to = buffer;
    for (size_t i = 0; i < len; i++) {
        to[i] = from[i];
    }

    return 0;
}

int read_block_index(struct ext2* fs, int level, size_t block) {

    if (fs->blockIndexAddress[level] != block) {
        fs->blockIndexAddress[level] = block;
        return read_block(fs, fs->blockIndexAddress[level], fs->blockIndexBuffer[level]);
    }

    return 0;
}

int ext2_write_inode_content(struct fs_Inode* inode, size_t offset, size_t size, void* buffer) {


}

size_t block_index_to_block(struct ext2* fs, struct ext2_Inode* inode, size_t blockIndex) {

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

    return block;
}



