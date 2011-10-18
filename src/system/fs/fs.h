#ifndef __SYSTEM_FS__
#define __SYSTEM_FS__

#include "system/fs/inode.h"
#include "type.h"

struct fs_Inode* fs_inode_open(size_t inode);

void fs_inode_close(struct fs_Inode* inode);

struct fs_Inode* fs_root(void);

int fs_load(void);

#endif
