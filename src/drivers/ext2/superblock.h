#ifndef __SUPERBLOCK__
#define __SUPERBLOCK__

#include "drivers/ext2/internal.h"

int ext2_superblock_init(struct ext2* fs);

int ext2_superblock_end(struct ext2* fs);

int ext2_get_total_block_groups(struct Superblock* superblock);

int ext2_superblock_write(struct ext2* fs);

#endif
