#ifndef __FS_INODE__
#define __FS_INODE__

#include "drivers/ext2/ext2.h"
#include "drivers/ext2/inode.h"
#include "type.h"

struct fs_inode {
    int number;
    struct ext2_Inode* data;
    struct ext2* fileSystem;
};

struct fs_inode* fs_read_inode(size_t number);
int fs_read_inode_content(struct fs_inode* inode, size_t offset, size_t size, void* buffer);
int fs_load(unsigned long long startSector);

#endif
