#ifndef __EXT2_BLOCK_GROUP__
#define __EXT2_BLOCK_GROUP__

struct BlockGroupDescriptor {
    unsigned int blockBitmapAddress;
    unsigned int inodeBitmapAddress;
    unsigned int inodeTableStart;
    unsigned short unallocatedBlocks;
    unsigned short unalloactedInodes;
    unsigned short directoryCount;
    unsigned char padding[14];
};

#endif
