#ifndef __SYSTEM_FS__
#define __SYSTEM_FS__

#include "system/fs/inode.h"
#include "system/fs/fd.h"
#include "type.h"

struct fs_Inode* fs_inode_open(size_t inode);

void fs_inode_close(struct fs_Inode* inode);

struct fs_Inode* fs_root(void);

int fs_load(void);

struct FileDescriptor fs_fd(struct fs_Inode* inode, int flags);

void fs_register_ops(int fileType, struct FileDescriptorOps ops);

#endif