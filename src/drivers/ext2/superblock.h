#ifndef __SUPERBLOCK__
#define __SUPERBLOCK__

#include "drivers/ext2/internal.h"

/**
 * Load the superblock for fs from disk.
 *
 * @param fs The file system.
 *
 * @return 0 on success, -1 on error
 */
int ext2_superblock_init(struct ext2* fs);

/**
 * Close & clean up the superblock.
 *
 * @param fs The fs.
 *
 * @return 0 on success, -1 on error
 */
int ext2_superblock_end(struct ext2* fs);

/**
 * Calculate the number of block groups.
 *
 * @param superblock The superblock to get the necesary data from.
 *
 * @return The number of block groups.
 */
int ext2_get_total_block_groups(struct Superblock* superblock);

/**
 * Write the superblock back to disk.
 *
 * @param fs The file system.
 *
 * @return 0 on success, -1 on error
 */
int ext2_superblock_write(struct ext2* fs);

#endif
