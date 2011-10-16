#ifndef __EXT2_BLOCK_GROUP__
#define __EXT2_BLOCK_GROUP__

#include "drivers/ext2/internal.h"

int ext2_read_blockgroup_table(struct ext2* fs);

int ext2_write_blockgroup_table(struct ext2* fs);

#endif
