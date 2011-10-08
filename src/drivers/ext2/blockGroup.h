#ifndef __EXT2_BLOCK_GROUP__
#define __EXT2_BLOCK_GROUP__

#include "type.h"

struct BlockGroupDescriptor {
    unsigned int blockBitmapAddress;
    unsigned int inodeBitmapAddress;
    unsigned int inodeTableStart;
    unsigned short unallocatedBlocks;
    unsigned short unalloactedInodes;
    unsigned short directoryCount;
    unsigned char padding[14];
};

struct BlockGroupDescriptorTable {
    size_t length;
    struct BlockGroupDescriptor descriptors[];
};

struct BlockGroupDescriptorTable* ext2_read_blockgroup_table(struct Superblock* sb);

#endif
