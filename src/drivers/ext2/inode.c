#include "drivers/ext2/inode.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"
#include "system/mm.h"
#include "system/fs/inode.h"
#include "system/call.h"
#include "drivers/ext2/allocator.h"

/**
 * Read a fragment of an inode, given the block index.
 *
 * The fragment to read from must be between block boundaries.
 *
 * @param inode The inode to read from.
 * @param blockIndex The index of the block in the inode block pointer table.
 * @param buffer The buffer to read to.
 * @param offset The offset to start reading from
 * @param len The number of bytes to read.
 *
 * @return 0 on success, -1 on error.
 */
static int read_inode_block(struct fs_Inode* inode, size_t blockIndex, void* buffer, size_t offset, size_t len);

/**
 * Read the block pointer index at block, to the index cache level.
 *
 * @param fs The current fs.
 * @param level The index level (ranges from 0 to 2)
 * @param block The physical block number to read from.
 *
 * @return 0 on success, -1 on error.
 */
static int read_block_index(struct ext2* fs, int level, size_t block);

/**
 * Write the block index at level back to disk.
 *
 * @param fs The current fs.
 * @param level The level to write.
 *
 * @return 0 on success, -1 on error.
 */
static int write_block_index(struct ext2* fs, int level);

static size_t block_index_to_block(struct ext2* fs, struct ext2_Inode* inode, size_t blockIndex);

/**
 * Load a block into one of the data block buffers.
 *
 * @param inode The inode the block belongs to.
 * @param block The block to load.
 *
 * @return The buffer index used, -1 on error
 */
static int load_buffer(struct fs_Inode* inode, size_t block);

/**
 * Alocate a new data block and set it as the block in index.
 *
 * @param inode The inode to add it to.
 * @param blockIndex The index in the block index table to set the new block to.
 *
 * @return The block number on success, 0 on error.
 */
static size_t allocate_data_block(struct fs_Inode* inode, size_t blockIndex);

/**
 * Free the blocks pointed to by table.
 *
 * @param fs The current fs.
 * @param table A table of allocated blocks.
 * @param size The number of entries in the table.
 *
 * @return 0 on success, -1 on error.
 */
static int free_blocks(struct ext2* fs, unsigned int* table, size_t size);

/**
 * Set all the entries in index to 0.
 *
 * @param fs The current fs.
 * @param level The level the index belongs to.
 * @param block The block to clear.
 *
 * @return 0 on success, -1 on error.
 */
static int clear_block_index(struct ext2* fs, int level, size_t block);

/**
 * Free all blocks associated with an index.
 *
 * @param fs The current fs.
 * @param indexBlock The block the index is on.
 * @param level The index level to block belongs to.
 * @param maxLevel The maximum level of indirection in this index tree.
 *
 * @return 0 on success, -1 on error.
 */
static int free_index(struct ext2* fs, unsigned int indexBlock, int level, int maxLevel);

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

    ext2_write_inode(fs_inode);

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

    // Try to find an entry we own
    for (bufferIndex = 0 ; bufferIndex < BLOCK_BUFFER_COUNT; bufferIndex++) {
        if (inode->fileSystem->blockBufferOwner[bufferIndex] == inode->number) {
            break;
        }
    }

    if (bufferIndex == BLOCK_BUFFER_COUNT) {

        // If we didnt find an entry, try to find one owned by no one

        for (bufferIndex = 0; bufferIndex < BLOCK_BUFFER_COUNT; bufferIndex++) {
            if (inode->fileSystem->blockBufferOwner[bufferIndex] == 0) {
                break;
            }
        }

        if (bufferIndex == BLOCK_BUFFER_COUNT) {

            // Otherwise, evict one
            inode->fileSystem->evictBlockBuffer++;
            inode->fileSystem->evictBlockBuffer %= BLOCK_BUFFER_COUNT;

            bufferIndex = inode->fileSystem->evictBlockBuffer;
        }

        // Reset the buffer, and mark it as ours

        inode->fileSystem->blockBufferOwner[bufferIndex] = inode->number;
        inode->fileSystem->blockBufferAddress[bufferIndex] = 0;
    }

    if (inode->fileSystem->blockBufferAddress[bufferIndex] != block) {

        // If the buffer didnt contain the block, already, load it

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

int ext2_write_inode_content(struct fs_Inode* inode, size_t offset, size_t size, const void* buffer) {

    struct ext2_Inode* node = inode->data;
    struct ext2* fs = inode->fileSystem;

    size_t firstBlockIndex = offset / fs->blockSize;
    size_t endBlockIndex = (offset + size) / fs->blockSize;

    size_t remainingBytes = size;
    const unsigned char* from = buffer;

    // If the offset is after the file end, let's make sure we fill in any blocks in the middle
    for (size_t blockIndex = node->size / fs->blockSize; blockIndex < firstBlockIndex; blockIndex++) {
        size_t block = block_index_to_block(fs, node, blockIndex);
        if (block == 0) {
            block = allocate_data_block(inode, blockIndex);
            if (block == 0) {
                break;
            }
        }
    }

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
            block = allocate_data_block(inode, blockIndex);
            if (block == 0) {
                break;
            }
        }

        if (startInBlock > 0 || remainingBytes < fs->blockSize) {

            // We gotta read the block, modify it, and write it

            size_t bufferIndex = load_buffer(inode, block);
            if (bufferIndex == -1) {
                break;
            }
            unsigned char* to = fs->blockBuffer[bufferIndex];

            size_t end = fs->blockSize;
            if (startInBlock + remainingBytes < fs->blockSize) {
                end = remainingBytes + startInBlock;
            }

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

    size_t writtenBytes = size - remainingBytes;
    if (offset + writtenBytes > node->size) {
        node->size = offset + writtenBytes;
    }

    ext2_write_inode(inode);
    fs->sb->lastWrittenTime = node->lastModification;

    ext2_superblock_write(fs);

    return writtenBytes;
}

size_t allocate_data_block(struct fs_Inode* inode, size_t blockIndex) {

    struct ext2_Inode* node = inode->data;
    struct ext2* fs = inode->fileSystem;

    size_t pointersPerBlock = fs->blockSize / sizeof(size_t);

    size_t blockGroup = inode->number / fs->sb->inodesPerBlockGroup;

    // Get a new block
    size_t newBlock = allocate_block(fs, blockGroup);
    if (newBlock == 0) {
        return 0;
    }

    // Once we have add it to the index, and allocate any index blocks if needed
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

int ext2_write_inode(struct fs_Inode* inode) {

    struct ext2* fs = inode->fileSystem;

    size_t blockGroup = inode->number / fs->sb->inodesPerBlockGroup;
    size_t index = (inode->number - 1) % fs->sb->inodesPerBlockGroup;

    size_t offsetInTable = sizeof(struct ext2_Inode) * index;
    size_t tableBlock = offsetInTable / fs->blockSize;
    inode->data->lastModification = _time(NULL);

    return write_block_fragment(
        fs,
        fs->groupTable[blockGroup].inodeTableStart + tableBlock,
        offsetInTable % fs->blockSize,
        sizeof(struct ext2_Inode),
        inode->data
    );
}

struct fs_Inode* ext2_create_inode(struct ext2* fs, int type, int permissions, int uid, int gid) {

    size_t number = allocate_inode(fs);
    if (number == 0) {
        return NULL;
    }

    struct fs_Inode* inode = ext2_read_inode(fs, number);
    if (inode == NULL) {
        return NULL;
    }

    for (int i = 0; i < 12; i++) {
        inode->data->directBlockPointers[i] = 0;
    }
    inode->data->singlyIndirectBlockPointer = 0;
    inode->data->doublyIndirectBlockPointer = 0;
    inode->data->triplyIndirectBlockPointer = 0;

    inode->data->countDiskSectors = 0;
    inode->data->size = 0;
    inode->data->hardLinks = 0;

    size_t now = _time(NULL);
    inode->data->lastAccess = now;
    inode->data->creationTime = now;
    inode->data->lastModification = now;
    inode->data->deletionTime = 0;

    inode->data->uid = uid;
    inode->data->gid = gid;
    inode->data->typesAndPermissions = ((unsigned int)type << 12) | (permissions & 0xFFF);

    ext2_write_inode(inode);
    return inode;
}

int ext2_delete_inode(struct fs_Inode* inode) {

    inode->data->deletionTime = _time(NULL);
    ext2_write_inode(inode);

    //FIXME: Hack :D
    if (INODE_TYPE(inode->data) == INODE_DIR) {
        struct ext2* fs = inode->fileSystem;
        size_t blockGroup = inode->number / fs->sb->inodesPerBlockGroup;
        fs->groupTable[blockGroup].directoryCount--;
        ext2_write_blockgroup_table(fs);
    }

    struct ext2_Inode* node = inode->data;
    struct ext2* fs = inode->fileSystem;

    free_blocks(fs, node->directBlockPointers, 12);
    free_index(fs, node->singlyIndirectBlockPointer, 0, 1);
    free_index(fs, node->doublyIndirectBlockPointer, 0, 2);
    free_index(fs, node->triplyIndirectBlockPointer, 0, 3);

    return deallocate_inode(inode->fileSystem, inode->number);
}

int free_index(struct ext2* fs, unsigned int indexBlock, int level, int maxLevel) {

    if (indexBlock == 0) {
        return 0;
    }

    if (read_block_index(fs, level, indexBlock) == -1) {
        return -1;
    }

    size_t entries = fs->blockSize / sizeof(unsigned int);

    if (level + 1 < maxLevel) {
        for (size_t i = 0; i < entries; i++) {

            // If the entry is not empty, recursively free all the data it points to.
            if (fs->blockIndexBuffer[level][i] != 0) {
                free_index(fs, fs->blockIndexBuffer[level][i], level + 1, maxLevel);
            } else {
                break;
            }
        }
    }

    // Free myself
    if (free_blocks(fs, fs->blockIndexBuffer[level], entries) == -1) {
        return -1;
    }

    return deallocate_block(fs, indexBlock);
}

int free_blocks(struct ext2* fs, unsigned int* table, size_t size) {

    for (size_t i = 0; i < size; i++) {

        if (table[i] != 0) {
            deallocate_block(fs, table[i]);
        } else {
            break;
        }
    }

    return 0;
}

