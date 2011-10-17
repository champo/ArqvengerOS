#ifndef __EXT2_BLOCK_GROUP__
#define __EXT2_BLOCK_GROUP__

#include "drivers/ext2/internal.h"

/**
 * Load the block group descriptor table.
 *
 * @param fs The current fs.
 *
 * @return 0 on success, -1 on error.
 */
int ext2_read_blockgroup_table(struct ext2* fs);

/**
 * Write back the block group descriptor table.
 *
 * @param fs The current fs.
 *
 * @return 0 on success, -1 on error.
 */
int ext2_write_blockgroup_table(struct ext2* fs);

#endif
