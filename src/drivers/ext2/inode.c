#include "drivers/ext2/inode.h"
#include "drivers/ext2/superblock.h"
#include "system/mm.h"
#include "system/fs/inode.h"
#include "system/call.h"
#include "drivers/ext2/allocator.h"

static int read_inode_block(struct fs_Inode* inode, size_t blockIndex, void* buffer, size_t offset, size_t len);

static int read_block_index(struct ext2* fs, int level, size_t block);

static int write_block_index(struct ext2* fs, int level);

static size_t block_index_to_block(struct ext2* fs, struct ext2_Inode* inode, size_t blockIndex);

static int load_buffer(struct fs_Inode* inode, size_t block);

static size_t allocate_data_block(struct fs_Inode* inode, size_t blockIndex);

static int write_inode(struct fs_Inode* inode);

static int clear_block_index(struct ext2* fs, int level, size_t block);

struct fs_Inode* ext2_read_inode(struct ext2* fs, size_t number) {

    if (fs->sb->totalInodes < number) {
        return NULL;
    }

    size_t blockGroup = number / fs->sb->inodesPerBlockGroup;
    size_t index = (number - 1) % fs->sb->inodesPerBlockGroup;

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
    fs_inode->data->lastAccess = _time(NULL);

    write_inode(fs_inode);

    return fs_inode;
}

int ext2_read_inode_content(struct fs_Inode* inode, size_t offset, size_t size, void* buffer) {

    if (offset >= inode->data->size) {
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

    return size;
}

int read_inode_block(struct fs_Inode* inode, size_t blockIndex, void* buffer, size_t offset, size_t len) {

    size_t block = block_index_to_block(inode->fileSystem, inode->data, blockIndex);
    int bufferIndex = load_buffer(inode, block);

    if (bufferIndex == -1) {
        return -1;
    }


    char* from = (char*) inode->fileSystem->blockBuffer[bufferIndex] + offset;
    char* to = buffer;
    for (size_t i = 0; i < len; i++) {
        to[i] = from[i];
    }

    return 0;
}

int load_buffer(struct fs_Inode* inode, size_t block) {

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

    return bufferIndex;
}

int write_block_index(struct ext2* fs, int level) {
    return write_block(fs, fs->blockIndexAddress[level], fs->blockIndexBuffer[level]);
}

int read_block_index(struct ext2* fs, int level, size_t block) {

    if (fs->blockIndexAddress[level] != block) {
        fs->blockIndexAddress[level] = block;
        return read_block(fs, fs->blockIndexAddress[level], fs->blockIndexBuffer[level]);
    }

    return 0;
}

int ext2_write_inode_content(struct fs_Inode* inode, size_t offset, size_t size, void* buffer) {

    struct ext2_Inode* node = inode->data;
    struct ext2* fs = inode->fileSystem;

    size_t firstBlockIndex = offset / fs->blockSize;
    size_t endBlockIndex = (offset + size) / fs->blockSize;

    size_t remainingBytes = size;
    unsigned char* from = buffer;

    for (size_t blockIndex = firstBlockIndex; blockIndex <= endBlockIndex; blockIndex++) {

        size_t blockOffset = blockIndex * fs->blockSize;
        size_t startInBlock;
        if (offset < blockOffset) {
            startInBlock = 0;
        } else {
            startInBlock = offset - blockOffset;
        }
        size_t block = block_index_to_block(fs, node, blockIndex);
        if (block == 0) {
            kprintf("Allocating block\n");
            block = allocate_data_block(inode, blockIndex);
            if (block == 0) {
                kprintf("Failed to get a new one, bailing\n");
                break;
            }
        }
        kprintf("Writing to block %u (%u) - offset %u remaining %u\n", blockIndex, block, startInBlock, remainingBytes);

        if (startInBlock > 0 || remainingBytes < fs->blockSize) {

            // We gotta read the block, modify it, and write it

            size_t bufferIndex = load_buffer(inode, block);
            unsigned char* to = fs->blockBuffer[bufferIndex];

            size_t end = fs->blockSize;
            if (startInBlock + remainingBytes < fs->blockSize) {
                end = remainingBytes + startInBlock;
            }
            kprintf("Buf idx %u end %u start %u\n", bufferIndex, end, startInBlock);

            for (size_t i = startInBlock; i < end; i++) {
                to[i] = *from;
                from++;
            }

            write_block(fs, block, fs->blockBuffer[bufferIndex]);
            remainingBytes -= end - startInBlock;
        } else {

            for (size_t i = 0; i < BLOCK_BUFFER_COUNT; i++) {
                if (fs->blockBufferAddress[i] == block) {
                    fs->blockBufferAddress[i] = 0;
                }
            }

            write_block(fs, block, from);
            from += fs->blockSize;
        }
    }

    node->lastModification = _time(NULL);
    size_t writtenBytes = size - remainingBytes;
    if (offset + writtenBytes > node->size) {
        node->size = offset + writtenBytes;
        kprintf("New size %u\n", node->size);
    }

    write_inode(inode);
    fs->sb->lastWrittenTime = node->lastModification;

    ext2_superblock_write(fs);

    return writtenBytes;
}

size_t allocate_data_block(struct fs_Inode* inode, size_t blockIndex) {

    struct ext2_Inode* node = inode->data;
    struct ext2* fs = inode->fileSystem;

    size_t pointersPerBlock = fs->blockSize / sizeof(size_t);

    size_t blockGroup = inode->number / fs->sb->inodesPerBlockGroup;

    size_t newBlock = allocate_block(fs, blockGroup);
    if (newBlock == 0) {
        kprintf("Got none\n");
        return 0;
    }

    if (blockIndex < 12) {
        node->directBlockPointers[blockIndex] = newBlock;
    } else if (blockIndex < 12 + pointersPerBlock) {

        if (node->singlyIndirectBlockPointer == 0) {

            node->singlyIndirectBlockPointer = allocate_block(fs, blockGroup);
            if (node->singlyIndirectBlockPointer == 0) {
                deallocate_block(fs, newBlock);
                return 0;
            }

            inode->data->countDiskSectors += fs->sectorsPerBlock;
        }

        read_block_index(fs, 0, node->singlyIndirectBlockPointer);
        fs->blockIndexBuffer[0][blockIndex - 12] = newBlock;
        write_block_index(fs, 0);

    } else if (blockIndex < 12 + pointersPerBlock + pointersPerBlock * pointersPerBlock) {

        size_t offsetInLevel = blockIndex - 12 - pointersPerBlock;
        size_t firstLevel = offsetInLevel / pointersPerBlock;
        size_t secondLevel = offsetInLevel % pointersPerBlock;

        if (node->doublyIndirectBlockPointer == 0) {

            node->doublyIndirectBlockPointer = allocate_block(fs, blockGroup);
            if (node->doublyIndirectBlockPointer == 0) {
                deallocate_block(fs, newBlock);
                return 0;
            }

            inode->data->countDiskSectors += fs->sectorsPerBlock;
        }

        read_block_index(fs, 0, node->doublyIndirectBlockPointer);

        if (fs->blockIndexBuffer[0][firstLevel] == 0) {

            fs->blockIndexBuffer[0][firstLevel] = allocate_block(fs, blockGroup);
            if (fs->blockIndexBuffer[0][firstLevel] == 0) {
                deallocate_block(fs, newBlock);
                return 0;
            }

            write_block_index(fs, 0);
            clear_block_index(fs, 1, fs->blockIndexBuffer[0][firstLevel]);
            inode->data->countDiskSectors += fs->sectorsPerBlock;
        }

        read_block_index(fs, 1, fs->blockIndexBuffer[0][firstLevel]);

        fs->blockIndexBuffer[1][secondLevel] = newBlock;
        write_block_index(fs, 1);

    } else {

        size_t pointersInSecondLevel = pointersPerBlock * pointersPerBlock;
        size_t offsetInLevel = blockIndex - 12 - pointersPerBlock - pointersInSecondLevel;

        size_t firstLevel = offsetInLevel / pointersInSecondLevel;
        size_t secondLevel = (offsetInLevel / pointersPerBlock) % pointersPerBlock;
        size_t thirdLevel = offsetInLevel % pointersPerBlock;

        if (node->triplyIndirectBlockPointer == 0) {

            node->triplyIndirectBlockPointer = allocate_block(fs, blockGroup);
            if (node->triplyIndirectBlockPointer == 0) {
                deallocate_block(fs, newBlock);
                return 0;
            }

            inode->data->countDiskSectors += fs->sectorsPerBlock;
        }

        read_block_index(fs, 0, node->triplyIndirectBlockPointer);

        if (fs->blockIndexBuffer[0][firstLevel] == 0) {

            fs->blockIndexBuffer[0][firstLevel] = allocate_block(fs, blockGroup);
            if (fs->blockIndexBuffer[0][firstLevel] == 0) {
                deallocate_block(fs, newBlock);
                return 0;
            }

            write_block_index(fs, 0);
            clear_block_index(fs, 1, fs->blockIndexBuffer[0][firstLevel]);
            inode->data->countDiskSectors += fs->sectorsPerBlock;
        }

        read_block_index(fs, 1, fs->blockIndexBuffer[0][firstLevel]);

        if (fs->blockIndexBuffer[1][secondLevel] == 0) {

            fs->blockIndexBuffer[1][secondLevel] = allocate_block(fs, blockGroup);
            if (fs->blockIndexBuffer[1][secondLevel] == 0) {
                deallocate_block(fs, newBlock);
                return 0;
            }

            write_block_index(fs, 1);
            clear_block_index(fs, 2, fs->blockIndexBuffer[1][secondLevel]);
            inode->data->countDiskSectors += fs->sectorsPerBlock;
        }

        read_block_index(fs, 2, fs->blockIndexBuffer[1][secondLevel]);

        fs->blockIndexBuffer[2][thirdLevel] = newBlock;
        write_block_index(fs, 2);
    }

    inode->data->countDiskSectors += fs->sectorsPerBlock;

    return newBlock;
}

int clear_block_index(struct ext2* fs, int level, size_t block) {

    read_block_index(fs, level, block);

    size_t entries = fs->blockSize / sizeof(unsigned int);
    for (size_t i = 0; i < entries; i++) {
        fs->blockIndexBuffer[level][i] = 0;
    }

    write_block_index(fs, level);

    return 0;
}

size_t block_index_to_block(struct ext2* fs, struct ext2_Inode* inode, size_t blockIndex) {

    size_t block;
    size_t pointersPerBlock = fs->blockSize / sizeof(size_t);

    if (blockIndex < 12) {
        block = inode->directBlockPointers[blockIndex];
    } else if (blockIndex - 12 < pointersPerBlock) {

        if (inode->singlyIndirectBlockPointer == 0) {
            return 0;
        }

        read_block_index(fs, 0, inode->singlyIndirectBlockPointer);
        block = fs->blockIndexBuffer[0][blockIndex - 12];

    } else if (blockIndex - 12 - pointersPerBlock < pointersPerBlock * pointersPerBlock) {

        size_t offsetInLevel = blockIndex - 12 - pointersPerBlock;
        size_t firstLevel = offsetInLevel / pointersPerBlock;
        size_t secondLevel = offsetInLevel % pointersPerBlock;

        if (inode->doublyIndirectBlockPointer == 0) {
            return 0;
        }

        read_block_index(fs, 0, inode->doublyIndirectBlockPointer);
        if (fs->blockIndexBuffer[0][firstLevel] == 0) {
            return 0;
        }

        read_block_index(fs, 1, fs->blockIndexBuffer[0][firstLevel]);

        block = fs->blockIndexBuffer[1][secondLevel];

    } else {

        size_t pointersInSecondLevel = pointersPerBlock * pointersPerBlock;
        size_t offsetInLevel = blockIndex - 12 - pointersPerBlock - pointersInSecondLevel;

        size_t firstLevel = offsetInLevel / pointersInSecondLevel;
        size_t secondLevel = (offsetInLevel / pointersPerBlock) % pointersPerBlock;
        size_t thirdLevel = offsetInLevel % pointersPerBlock;

        if (inode->triplyIndirectBlockPointer == 0) {
            return 0;
        }

        read_block_index(fs, 0, inode->triplyIndirectBlockPointer);
        if (fs->blockIndexBuffer[0][firstLevel] == 0) {
            return 0;
        }

        read_block_index(fs, 1, fs->blockIndexBuffer[0][firstLevel]);
        if (fs->blockIndexBuffer[1][secondLevel] == 0) {
            return 0;
        }

        read_block_index(fs, 2, fs->blockIndexBuffer[1][secondLevel]);

        block = fs->blockIndexBuffer[2][thirdLevel];
    }

    return block;
}

int write_inode(struct fs_Inode* inode) {

    struct ext2* fs = inode->fileSystem;

    size_t blockGroup = inode->number / fs->sb->inodesPerBlockGroup;
    size_t index = (inode->number - 1) % fs->sb->inodesPerBlockGroup;

    size_t offsetInTable = sizeof(struct ext2_Inode) * index;
    size_t tableBlock = offsetInTable / fs->blockSize;

    return write_block_fragment(
        fs,
        fs->groupTable[blockGroup].inodeTableStart + tableBlock,
        offsetInTable % fs->blockSize,
        sizeof(struct ext2_Inode),
        inode->data
    );
}

