#ifndef __SYSTEM_FS_FD__
#define __SYSTEM_FS_FD__

struct FileDescriptor;

struct FileDescriptorOps {
    size_t (*write)(struct FileDescriptor*, const void*, size_t);
    size_t (*read)(struct FileDescriptor*, void*, size_t);
    size_t (*ioctl)(struct FileDescriptor*, int, void*);
    int (*close)(struct FileDescriptor*);
};

struct FileDescriptor {
    struct fs_Inode* inode;

    int flags;
    size_t offset;

    struct FileDescriptorOps* ops;
};

#endif
