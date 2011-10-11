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

#endif
