#include "drivers/ext2/directory.h"
#include "system/fs/directory.h"
#include "library/string.h"
#include "drivers/ext2/internal.h"
#include "drivers/ext2/blockGroup.h"
#include "system/mm.h"

#define ENTRY_HEADER_LEN (sizeof(unsigned int) + 2 * sizeof(unsigned short))

inline static unsigned int pad(unsigned int value, unsigned int align) {

    unsigned int padding = align - (value % align);
    if (padding == align) {
        return value;
    } else {
        return value + padding;
    }
}

struct DirectoryEntry ext2_dir_read(struct fs_Directory* directory) {

    struct DirectoryEntry entry;

    if (INODE_TYPE(directory->inode->data) != INODE_DIR) {
        entry.entryLength = 0;
        return entry;
    }

    if (ext2_read_inode_content(directory->inode, directory->offset, sizeof(struct DirectoryEntry), &entry) == -1) {
        entry.entryLength = 0;
    }

    if (entry.entryLength != 0) {
        entry.name[entry.nameLength] = 0;
    }

    return entry;
}

struct DirectoryEntry ext2_dir_find(struct fs_Directory* directory, const char* name) {

    size_t oldOffset = directory->offset;
    directory->offset = 0;

    struct DirectoryEntry entry = ext2_dir_read(directory);
    while (entry.entryLength != 0) {

        if (strcmp(entry.name, name) == 0) {
            break;
        }

        directory->offset += entry.entryLength;
        entry = ext2_dir_read(directory);
    }

    directory->offset = oldOffset;
    return entry;
}

int ext2_dir_add(struct fs_Directory* directory, const char* name, size_t inodeNumber) {

    struct DirectoryEntry entry;
    struct fs_Inode* inode = directory->inode;
    struct ext2* fs = inode->fileSystem;

    if (ext2_dir_find(directory, name).entryLength != 0) {
        return -1;
    }

    entry.inode = inodeNumber;
    entry.nameLength = strlen(name);
    if (entry.nameLength > NAME_MAX_LEN) {
        entry.nameLength = NAME_MAX_LEN;
    }

    strncpy(entry.name, name, NAME_MAX_LEN);
    entry.name[NAME_MAX_LEN] = 0;

    size_t offset = 0;
    size_t len = ENTRY_HEADER_LEN + entry.nameLength * sizeof(char);

    // Align the length to a 4-byte boundary
    entry.entryLength = pad(len, 4);

    if (directory->inode->data->size == 0) {
        // The last entry in the a block has to span the whole block
        entry.entryLength = fs->blockSize;
    } else {

        size_t oldOffset = directory->offset;
        directory->offset = 0;

        struct DirectoryEntry cur = ext2_dir_read(directory);
        while (cur.entryLength != 0) {

            // Check if this entry has more space than it needs, and if we fit there

            size_t realLen = pad(ENTRY_HEADER_LEN + cur.nameLength, 4);
            if (realLen + len <= cur.entryLength) {

                // We got ourselves a winner!
                entry.entryLength = cur.entryLength - realLen;
                cur.entryLength = realLen;

                if (ext2_write_inode_content(inode, directory->offset, realLen, &cur) == -1) {
                    return -1;
                }

                offset = directory->offset + realLen;
                break;
            }

            directory->offset += cur.entryLength;
            cur = ext2_dir_read(directory);
        }

        if (offset == 0) {
            offset = directory->offset;
            entry.entryLength = fs->blockSize - (offset % fs->blockSize);
        }

        directory->offset = oldOffset;
    }

    if ((offset % fs->blockSize) + len >= fs->blockSize) {

        kprintf("Crossing boundary, old values: %u, %u\n", offset, entry.entryLength);

        /*
         * The entry crosses a block boundary, which cant happen
         * This has to mean that this will be the last entry in the directory
         * So we just adjust it to create a new block
         *
         * Why can't it be another case?
         *  - If the size is 0, it clearly cant be, since we cant have a name longer than a block
         *  - If we found an entry with enough empty space for us, since the last
         *  entry in a block always points to the end of that block (We always make sure of this)
         *  if there's enough space, it's in the same block.
         */

        offset = fs->blockSize * (1 + (offset / fs->blockSize));
        entry.entryLength = fs->blockSize;
    }

    kprintf("settings offset to %u, len %u\n", offset, entry.entryLength);
    if (ext2_write_inode_content(inode, offset, len, &entry) == -1) {
        return -1;
    }

    // A directory's size is always aligned to a block
    inode->data->size = pad(inode->data->size, fs->blockSize);

    return ext2_write_inode(inode);
}

int ext2_dir_remove(struct fs_Directory* directory, const char* name) {

    struct fs_Inode* inode = directory->inode;
    struct ext2* fs = inode->fileSystem;

    size_t oldOffset = directory->offset;
    directory->offset = 0;

    struct DirectoryEntry entry = ext2_dir_read(directory);
    size_t lastOffset = 0;

    while (entry.entryLength != 0) {

        if (strncmp(entry.name, name, entry.nameLength) == 0) {

            if ((directory->offset % fs->blockSize) == 0) {

                // If it's the first entry in a block, we just mark it as empty
                entry.inode = 0;
                entry.nameLength = 0;
            } else {

                // We adjust the length of the previous entry to cover this one
                size_t entryLen = entry.entryLength;
                directory->offset = lastOffset;

                entry = ext2_dir_read(directory);
                entry.entryLength += entryLen;
            }

            size_t realLen = pad(ENTRY_HEADER_LEN + entry.nameLength, 4);
            ext2_write_inode_content(inode, directory->offset, realLen, &entry);

            break;
        }

        lastOffset = directory->offset;
        directory->offset += entry.entryLength;

        entry = ext2_dir_read(directory);
    }

    directory->offset = oldOffset;
    return 0;
}

struct fs_Directory* ext2_dir_create(struct ext2* fs, int permissions, int uid, int gid) {

    struct fs_Inode* inode = ext2_create_inode(fs, INODE_DIR, permissions, uid, gid);
    if (inode == NULL) {
        return NULL;
    }

    struct fs_Directory* dir = kalloc(sizeof(struct fs_Directory));

    dir->inode = inode;
    dir->offset = 0;

    // Update the amount of directories in this block group
    size_t blockGroup = inode->number / fs->sb->inodesPerBlockGroup;
    fs->groupTable[blockGroup].directoryCount++;
    ext2_write_blockgroup_table(fs);

    return dir;
}

int ext2_dir_rename(struct fs_Directory* directory, const char* from, const char* to) {

    struct DirectoryEntry original = ext2_dir_find(directory, from);
    if (original.entryLength == 0) {
        return -1;
    }

    if (ext2_dir_remove(directory, from) == -1) {
        return -1;
    }

    return ext2_dir_add(directory, to, original.inode);
}

