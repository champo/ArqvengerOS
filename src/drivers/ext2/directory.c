#include "drivers/ext2/directory.h"

struct DirectoryEntry ext2_dir_read(struct ext2* fs, struct ext2_Inode* inode, size_t offset) {

    struct DirectoryEntry entry;

    if (INODE_TYPE(inode) != INODE_DIR) {
        entry.inode = 0;
        return entry;
    }

    if (ext2_read_inode_content(fs, inode, offset, sizeof(struct DirectoryEntry), &entry) == -1) {
        entry.inode = 0;
    }

    if (entry.inode != 0) {
        entry.name[entry.nameLength] = 0;
    }

    return entry;
}

