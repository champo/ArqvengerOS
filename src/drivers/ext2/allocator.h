#ifndef __EXT2_ALLOCATOR__
#define __EXT2_ALLOCATOR__

#include "drivers/ext2/internal.h"
#include "type.h"

size_t allocate_block(struct ext2* fs, int groupHint);

int deallocate_block(struct ext2* fs, size_t block);

#endif
