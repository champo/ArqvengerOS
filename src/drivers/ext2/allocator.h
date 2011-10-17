#ifndef __EXT2_ALLOCATOR__
#define __EXT2_ALLOCATOR__

#include "drivers/ext2/internal.h"
#include "type.h"

/**
 * Allocate a block, preferably in a given group.
 *
 * @param fs The current fs.
 * @param groupHint The preferred group number.
 *
 * @return The allocated block number on success, 0 otherwise.
 */
size_t allocate_block(struct ext2* fs, int groupHint);

/**
 * Deallocate a previously allocate block.
 *
 * @parm fs The current fs.
 * @param block The block number.
 *
 * @return 0 on success, -1 on error.
 */
int deallocate_block(struct ext2* fs, size_t block);

/**
 * Allocate an used inode.
 *
 * @param fs The current fs.
 *
 * @return The inode number on success, 0 otherwise.
 */
size_t allocate_inode(struct ext2* fs);

/**
 * Deallocate a previously allocated inode.
 *
 * @param fs The current fs.
 * @param inode The inode number.
 *
 * @return 0 on success, -1 on error
 */
int deallocate_inode(struct ext2* fs, size_t inode);

#endif
