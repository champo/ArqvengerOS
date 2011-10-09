#ifndef __EXT2_INTERNAL__
#define __EXT2_INTERNAL__

#include "type.h"
#include "drivers/ata.h"

#define SUPERBLOCK_SIZE 1024
#define SUPERBLOCK_SECTORS 2
#define SUPERBLOCK_START 2ull

//If extended fields needed, just ask!

struct Superblock {
    dword totalInodes;
    dword totalBlocks;
    dword blocksForSuperuser;
    dword unallocatedBlocks;
    dword unallocatedInodes;
    dword superblockBlockNumber;
    dword blockSize; //log2 (blocksize) -10 really.
    dword fragmentSize; //log2(fragmentsize) -10 really.
    dword blocksPerBlockGroup;
    dword fragmentsPerBlockGroup;
    dword inodesPerBlockGroup;
    dword lastMountTime; //Posix time.
    dword lastWrittenTime; //Posix time.
    word totalMountTimesSinceLastCheck;
    word mountsUntilNextCheck;
    word ext2Signature;
    word fileSystemState;
    word errorAction;
    word minorVersion;
    dword lastCheck; //Posixt time.
    dword intervalBetweenChecks; //Posix time.
    dword operatingSystemID;
    dword majorVersion;
    word userIDforReservedBlocks;
    word groupIDforReservedBlocks;
};

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

struct ext2 {
    struct Superblock* sb;
    struct BlockGroupDescriptorTable* groupTable;
    unsigned long long firstSector;
};

int read_sectors(struct ext2* fs, unsigned long long sector, size_t sectors, void* buffer);

#endif
