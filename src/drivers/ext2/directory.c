#include "drivers/ext2/directory.h"
#include "library/string.h"
#include "drivers/ext2/internal.h"
#include "drivers/ext2/blockGroup.h"
#include "system/mm.h"

#define ENTRY_HEADER_LEN (sizeof(unsigned int) + 2 * sizeof(unsigned short))

/**
 * Pad a size to a given aligment.
 *
 * @param value The size to pad.
 * @param align The desired aligment.
 *
 * @return the aligned size.
 */
inline static unsigned int pad(unsigned int value, unsigned int align) {

    unsigned int padding = align - (value % align);
    if (padding == align) {
        return value;
    } else {
        return value + padding;
    }
}

struct DirectoryEntry ext2_dir_read(struct fs_Inode* directory, size_t offset) {

    struct DirectoryEntry entry = {
        .entryLength = 0,
        .nameLength = 0,
        .inode = 0
    };

    if (INODE_TYPE(directory->data) != INODE_DIR) {
        return entry;
    }

    if (ext2_read_inode_content(directory, offset, sizeof(struct DirectoryEntry), &entry) == -1) {
        entry.entryLength = 0;
    }

    if (entry.entryLength != 0) {
        // It seems ext2 doesnt force a NULL at the end of strings, so we do
        entry.name[entry.nameLength] = 0;
    }

    return entry;
}

struct DirectoryEntry ext2_dir_find(struct fs_Inode* directory, const char* name) {

    size_t offset = 0;

    struct DirectoryEntry entry = ext2_dir_read(directory, offset);
    while (entry.entryLength != 0) {

        if (strcmp(entry.name, name) == 0) {
            break;
        }

        offset += entry.entryLength;
        entry = ext2_dir_read(directory, offset);
    }

    return entry;
}

int ext2_dir_add(struct fs_Inode* directory, const char* name, size_t inodeNumber) {

    struct DirectoryEntry entry;
    struct ext2* fs = directory->fileSystem;

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

    if (directory->data->size == 0) {
        // The last entry in the a block has to span the whole block
        entry.entryLength = fs->blockSize;
    } else {

        size_t dirOffset = 0;

        struct DirectoryEntry cur = ext2_dir_read(directory, dirOffset);
        while (cur.entryLength != 0) {

            // Check if this entry has more space than it needs, and if we fit there

            size_t realLen = pad(ENTRY_HEADER_LEN + cur.nameLength, 4);
            if (realLen + len <= cur.entryLength) {

                // We got ourselves a winner!
                entry.entryLength = cur.entryLength - realLen;
                cur.entryLength = realLen;

                if (ext2_write_inode_content(directory, dirOffset, realLen, &cur) == -1) {
                    return -1;
                }

                offset = dirOffset + realLen;
                break;
            }

            dirOffset += cur.entryLength;
            cur = ext2_dir_read(directory, dirOffset);
        }

        if (offset == 0) {
            offset = dirOffset;
            entry.entryLength = fs->blockSize - (offset % fs->blockSize);
        }
    }

    if ((offset % fs->blockSize) + len >= fs->blockSize) {

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

    if (ext2_write_inode_content(directory, offset, len, &entry) == -1) {
        return -1;
    }

    // A directory's size is always aligned to a block
    directory->data->size = pad(directory->data->size, fs->blockSize);

    return ext2_write_inode(directory);
}

int ext2_dir_remove(struct fs_Inode* directory, const char* name) {

    struct ext2* fs = directory->fileSystem;

    size_t offset = 0;

    struct DirectoryEntry entry = ext2_dir_read(directory, offset);
    size_t lastOffset = 0;

    while (entry.entryLength != 0) {

        if (strncmp(entry.name, name, entry.nameLength) == 0) {

            if ((offset % fs->blockSize) == 0) {

                // If it's the first entry in a block, we just mark it as empty
                entry.inode = 0;
                entry.nameLength = 0;
            } else {

                // We adjust the length of the previous entry to cover this one
                size_t entryLen = entry.entryLength;
                offset = lastOffset;

                entry = ext2_dir_read(directory, offset);
                entry.entryLength += entryLen;
            }

            size_t realLen = pad(ENTRY_HEADER_LEN + entry.nameLength, 4);
            ext2_write_inode_content(directory, offset, realLen, &entry);

            break;
        }

        lastOffset = offset;
        offset += entry.entryLength;

        entry = ext2_dir_read(directory, offset);
    }

    return 0;
}

struct fs_Inode* ext2_dir_create(struct ext2* fs, int permissions, int uid, int gid) {

    struct fs_Inode* inode = ext2_create_inode(fs, INODE_DIR, permissions, uid, gid);
    if (inode == NULL) {
        return NULL;
    }

    // Update the amount of directories in this block group
    size_t blockGroup = inode->number / fs->sb->inodesPerBlockGroup;
    fs->groupTable[blockGroup].directoryCount++;
    ext2_write_blockgroup_table(fs);

    return inode;
}

int ext2_dir_rename(struct fs_Inode* directory, const char* from, const char* to) {

    struct DirectoryEntry original = ext2_dir_find(directory, from);
    if (original.entryLength == 0) {
        return -1;
    }

    if (ext2_dir_remove(directory, from) == -1) {
        return -1;
    }

    return ext2_dir_add(directory, to, original.inode);
}

