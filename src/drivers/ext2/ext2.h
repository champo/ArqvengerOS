#ifndef __EXT2__
#define __EXT2__

struct ext2;

struct ext2_Inode;

/**
 * Load an ext2 file system.
 *
 * @param startSector The initial disk sector the fs is located on.
 *
 * @return The structure on success, NULL on error
 */
struct ext2* ext2_load(unsigned long long startSector);

#endif
