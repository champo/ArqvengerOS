#ifndef __SYSTEM_FS_FD__
#define __SYSTEM_FS_FD__

#include "system/fs/direntry.h"
#include "system/fs/inode.h"

struct FileDescriptor;

struct FileDescriptorOps {
    void (*open)(struct FileDescriptor*);
    size_t (*write)(struct FileDescriptor*, const void*, size_t);
    size_t (*read)(struct FileDescriptor*, void*, size_t);
    int (*ioctl)(struct FileDescriptor*, int, void*);
    struct fs_DirectoryEntry (*readdir)(struct FileDescriptor*);
    int (*close)(struct FileDescriptor*);
};

struct FileDescriptor {
    struct fs_Inode* inode;

    int flags;
    size_t offset;

    struct FileDescriptorOps* ops;
};

#endif
