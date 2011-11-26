#ifndef __SYSTEM_FS__
#define __SYSTEM_FS__

#include "system/fs/inode.h"
#include "system/fs/fd.h"
#include "type.h"
#include "constants.h"

#define PERM_DEFAULT 0744

struct fs_Inode* fs_inode_open(size_t inode);

void fs_inode_close(struct fs_Inode* inode);

struct fs_Inode* fs_root(void);

struct fs_DirectoryEntry fs_findentry(struct fs_Inode* path, const char* name);

int fs_mknod(struct fs_Inode* inode, const char* name, int type);

int fs_mkdir(struct fs_Inode* path, const char* name);

int fs_rmdir(struct fs_Inode* path, const char* name);

int fs_unlink(struct fs_Inode* path, const char* name);

int fs_symlink(struct fs_Inode* path, const char* entry, const char* to);

char* fs_symlink_read(struct fs_Inode* symlink, int size, char* buff);

int fs_get_inode_size(struct fs_Inode* inode);

int fs_rename(struct fs_Inode* from, const char* original, struct fs_Inode* to, const char* new);

int fs_permission(struct fs_Inode* inode);

int fs_set_permission(struct fs_Inode* inode, int perm);

int fs_load(void);

void fs_fd(struct FileDescriptor* to, struct fs_Inode* inode, int flags);

void fs_dup(struct FileDescriptor* to, struct FileDescriptor fd);

int fs_fd_close(struct FileDescriptor* fd);

void fs_register_ops(int fileType, struct FileDescriptorOps ops);

int fs_set_own(struct fs_Inode* inode, int uid, int gid);

#endif
