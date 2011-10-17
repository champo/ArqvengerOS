#ifndef __FS_DIRECTORY__
#define __FS_DIRECTORY__

#include "system/fs/inode.h"
#include "type.h"

struct fs_Directory {
    struct fs_Inode* inode;
    size_t offset;
};

#endif
