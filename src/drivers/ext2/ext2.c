#include "drivers/ext2/ext2.h"
#include "drivers/ext2/internal.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"
#include "system/mm.h"
#include "system/kprintf.h"
#include "drivers/ext2/inode.h"
#include "drivers/ext2/directory.h"

struct ext2* ext2_load(unsigned long long startSector) {

    struct ext2* fs = kalloc(sizeof(struct ext2));

    fs->firstSector = startSector;

    ext2_superblock_init(fs);

    fs->blockSize = 1024 << fs->sb->blockSize;
    fs->sectorsPerBlock = fs->blockSize / SECTOR_SIZE;
    fs->blockGroupCount = ext2_get_total_block_groups(fs->sb);

    for (int i = 0; i < 3; i++) {
        fs->blockIndexAddress[i] = 0;
        fs->blockIndexBuffer[i] = kalloc(fs->blockSize);
    }

    fs->fragmentReadBlock = 0;
    fs->fragmentReadBuffer = kalloc(fs->blockSize);

    fs->evictBlockBuffer = 0;
    for (int i = 0; i < BLOCK_BUFFER_COUNT; i++) {
        fs->blockBufferAddress[i] = 0;
        fs->blockBufferOwner[i] = NULL;
        fs->blockBuffer[i] = kalloc(fs->blockSize);
    }

    ext2_read_blockgroup_table(fs);

    kprintf("Block size %u Sectors per block %u\n", fs->blockSize, fs->sectorsPerBlock);
    for (size_t i = 0; i < fs->blockGroupCount; i++) {
        kprintf("Block Group %d: free blocks %u, table start %u\n", i, fs->groupTable[i].unallocatedBlocks, fs->groupTable[i].inodeTableStart);
    }

    struct ext2_Inode* root = ext2_read_inode(fs, 2);
    kprintf("Root dir size %u type %u\n", root->size, INODE_TYPE(root));
    size_t offset = 0;
    struct DirectoryEntry entry = ext2_dir_read(fs, root, offset);
    while (entry.inode != 0) {
        offset += entry.entryLength;
        kprintf("Entry %s(%u) inode %u\n", entry.name, entry.nameLength, entry.inode);
        entry = ext2_dir_read(fs, root, offset);
    }

    return fs;
}

