#ifndef __INODE__
#define __INODE__

#include "drivers/ext2/inode.h"
#include "system/fs/fs.h"
#include "type.h"

struct Inode {
    int number;
    struct ext2_Inode* data;
};

struct Inode* read_inode(struct Filesystem* fs, size_t number);
int read_inode_content(struct Filesystem* fs, struct Inode* inode, size_t offset, size_t size, void* buffer);

#endif
