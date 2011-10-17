#ifndef __EXT2__DIRECTORY__
#define __EXT2__DIRECTORY__

#include "type.h"
#include "drivers/ext2/internal.h"
#include "drivers/ext2/inode.h"
#include "system/fs/directory.h"

#define NAME_MAX_LEN 256

struct DirectoryEntry {
    unsigned int inode;
    unsigned short entryLength;
    unsigned short nameLength;
    char name[NAME_MAX_LEN + 1];
};

struct DirectoryEntry ext2_dir_read(struct fs_Directory* directory);

struct DirectoryEntry ext2_dir_find(struct fs_Directory* directory, const char* name);

int ext2_dir_add(struct fs_Directory* directory, const char* name, size_t inodeNumber);

int ext2_dir_remove(struct fs_Directory* directory, const char* name);

int ext2_dir_rename(struct fs_Directory* directory, const char* from, const char* to);

struct fs_Directory* ext2_dir_create(struct ext2* fs, int permissions, int uid, int gid);

#endif
