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

/**
 * Read the directory entry at the current offset.
 *
 * @param directory The directory to read from.
 *
 * @return The entry if one exists, otherwise the field entryLenght is set to 0.
 */
struct DirectoryEntry ext2_dir_read(struct fs_Directory* directory);

/**
 * Find an entry with a given name.
 *
 * @param directory The directory to read from.
 * @param name The name to look for.
 *
 * @return The entry if it exists, otherwie the field entryLength is set to 0.
 */
struct DirectoryEntry ext2_dir_find(struct fs_Directory* directory, const char* name);

/**
 * Add a new entry to the directory. If it already exists, it's consired a success.
 *
 * @param directory The directory to write to.
 * @param name The entrys name.
 * @param inodeNumber The inode to point to.
 *
 * @return 0 on success, -1 on failure.
 */
int ext2_dir_add(struct fs_Directory* directory, const char* name, size_t inodeNumber);

/**
 * Remove an entry from the directory.
 *
 * @param directory The directory to write to.
 * @param name The name of the entry to remove.
 *
 * @return 0 on success, -1 on failure.
 */
int ext2_dir_remove(struct fs_Directory* directory, const char* name);

/**
 * Rename an entry.
 *
 * @param directory The directory to modify.
 * @param from The name of the entry.
 * @param to The new name for the entry.
 *
 * @return 0 on success, -1 on failure.
 */
int ext2_dir_rename(struct fs_Directory* directory, const char* from, const char* to);

/**
 * Create a new directory.
 *
 * @param fs The file system that will hold the directory.
 * @param permissions The permissions for the directory.
 * @param uid The id of the owner.
 * @param gid The id of the owning group.
 *
 * @return The entry on success, NULL on failure.
 */
struct fs_Directory* ext2_dir_create(struct ext2* fs, int permissions, int uid, int gid);

#endif
