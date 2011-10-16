#include "drivers/ext2/ext2.h"
#include "drivers/ext2/internal.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"
#include "system/mm.h"
#include "system/kprintf.h"
#include "drivers/ext2/inode.h"
#include "drivers/ext2/directory.h"
#include "system/fs/directory.h"
#include "system/fs/inode.h"

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

    fs->bitmapBlock = 0;
    fs->bitmapBuffer = kalloc(fs->blockSize);

    ext2_read_blockgroup_table(fs);

    kprintf("Block size %u Sectors per block %u Blocks per group %u\n", fs->blockSize, fs->sectorsPerBlock, fs->sb->blocksPerBlockGroup, fs->sb->inodesPerBlockGroup);
    for (size_t i = 0; i < fs->blockGroupCount; i++) {
        kprintf("Group %d: free blocks %u, table start %u, bitmap %u\n", i, fs->groupTable[i].unallocatedBlocks, fs->groupTable[i].inodeTableStart,
                fs->groupTable[i].blockBitmapAddress);
    }

    struct fs_Directory* dir = kalloc(sizeof(struct fs_Directory));
    dir->offset = 0;
    dir->inode = ext2_read_inode(fs, 2);
    kprintf("Root dir size %u type %u\n", dir->inode->data->size, INODE_TYPE(dir->inode->data));
    struct DirectoryEntry entry = ext2_dir_read(dir);
    while (entry.inode != 0) {
        dir->offset += entry.entryLength;
        kprintf("Entry %s(%u) inode %u\n", entry.name, entry.nameLength, entry.inode);
        entry = ext2_dir_read(dir);
    }

    dir->inode = ext2_read_inode(fs, 16);
    kprintf("test dir size %u type %u\n", dir->inode->data->size, INODE_TYPE(dir->inode->data));
    dir->offset = 0;
    entry = ext2_dir_read(dir);
    size_t lastNode;
    while (entry.inode != 0) {
        dir->offset += entry.entryLength;
        kprintf("Entry %s(%u) inode %u\n", entry.name, entry.nameLength, entry.inode);
        lastNode = entry.inode;
        entry = ext2_dir_read(dir);
    }

    struct fs_Inode* plist = ext2_read_inode(fs, lastNode);
    char* buff = kalloc(1200);
    ext2_read_inode_content(plist, 0, 4, buff);
    kprintf("%u %u %u %u\n", buff[0], buff[1], buff[2], buff[3]);
    kprintf("%u\n", plist->data->size);
    ext2_read_inode_content(plist, 0, 1200, buff);
    for (int i = 0; i < 1200; i++) {
        if (buff[i] != '0' + (i % 10)) kprintf("CRAP");
    }
    return fs;
}

