#ifndef __SYSTEM_FS_DIRENTRY__
#define __SYSTEM_FS_DIRENTRY__

#define ENTRY_NAME_MAX_LEN 256

struct fs_DirectoryEntry {
    size_t inode;
    size_t length;
    char name[ENTRY_NAME_MAX_LEN];
};

#endif
