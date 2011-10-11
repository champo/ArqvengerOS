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

    struct fs_Directory* dir = kalloc(sizeof(struct fs_Directory));
    dir->inode = kalloc(sizeof(struct fs_Inode));
    dir->offset = 0;
    dir->inode->fileSystem = kalloc(sizeof(struct ext2));

    dir->inode->fileSystem->firstSector = startSector;

    ext2_superblock_init(dir->inode->fileSystem);

    dir->inode->fileSystem->blockSize = 1024 << dir->inode->fileSystem->sb->blockSize;
    dir->inode->fileSystem->sectorsPerBlock = dir->inode->fileSystem->blockSize / SECTOR_SIZE;
    dir->inode->fileSystem->blockGroupCount = ext2_get_total_block_groups(dir->inode->fileSystem->sb);

    for (int i = 0; i < 3; i++) {
        dir->inode->fileSystem->blockIndexAddress[i] = 0;
        dir->inode->fileSystem->blockIndexBuffer[i] = kalloc(dir->inode->fileSystem->blockSize);
    }

    dir->inode->fileSystem->fragmentReadBlock = 0;
    dir->inode->fileSystem->fragmentReadBuffer = kalloc(dir->inode->fileSystem->blockSize);

    dir->inode->fileSystem->evictBlockBuffer = 0;
    for (int i = 0; i < BLOCK_BUFFER_COUNT; i++) {
        dir->inode->fileSystem->blockBufferAddress[i] = 0;
        dir->inode->fileSystem->blockBufferOwner[i] = NULL;
        dir->inode->fileSystem->blockBuffer[i] = kalloc(dir->inode->fileSystem->blockSize);
    }

    ext2_read_blockgroup_table(dir->inode->fileSystem);

    kprintf("Block size %u Sectors per block %u\n", dir->inode->fileSystem->blockSize, dir->inode->fileSystem->sectorsPerBlock);
    for (size_t i = 0; i < dir->inode->fileSystem->blockGroupCount; i++) {
        kprintf("Block Group %d: free blocks %u, table start %u\n", i, dir->inode->fileSystem->groupTable[i].unallocatedBlocks, dir->inode->fileSystem->groupTable[i].inodeTableStart);
    }

    dir->inode->data = ext2_read_inode(dir->inode->fileSystem, 2);
    kprintf("Root dir size %u type %u\n", dir->inode->data->size, INODE_TYPE(dir->inode->data));
    struct DirectoryEntry entry = ext2_dir_read(dir);
    while (entry.inode != 0) {
        dir->offset += entry.entryLength;
        kprintf("Entry %s(%u) inode %u\n", entry.name, entry.nameLength, entry.inode);
        entry = ext2_dir_read(dir);
    }

    return dir->inode->fileSystem;
}

