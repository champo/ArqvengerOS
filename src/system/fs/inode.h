#ifndef __FS_INODE__
#define __FS_INODE__

#include "drivers/ext2/ext2.h"
#include "drivers/ext2/inode.h"
#include "system/lock/mutex.h"

struct fs_Inode {
    unsigned int number;

    struct ext2_Inode* data;
    struct ext2* fileSystem;

    size_t refCount;

    void* extra;
    struct Mutex lock;
};

#endif
