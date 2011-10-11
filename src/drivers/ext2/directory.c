#include "drivers/ext2/directory.h"
#include "system/fs/directory.h"

struct DirectoryEntry ext2_dir_read(struct fs_Directory* directory) {

    struct DirectoryEntry entry;

    if (INODE_TYPE(directory->inode->data) != INODE_DIR) {
        entry.inode = 0;
        return entry;
    }

    if (ext2_read_inode_content(directory->inode, directory->offset, sizeof(struct DirectoryEntry), &entry) == -1) {
        entry.inode = 0;
    }

    if (entry.inode != 0) {
        entry.name[entry.nameLength] = 0;
    }

    return entry;
}

