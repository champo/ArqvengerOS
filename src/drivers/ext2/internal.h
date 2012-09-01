#ifndef __EXT2_INTERNAL__
#define __EXT2_INTERNAL__

#include "type.h"
#include "drivers/ata.h"

#define SUPERBLOCK_SIZE 1024
#define SUPERBLOCK_SECTORS 2
#define SUPERBLOCK_START 2ull

//If extended fields needed, just ask!

struct Superblock {
    unsigned int totalInodes;
    unsigned int totalBlocks;
    unsigned int blocksForSuperuser;
    unsigned int unallocatedBlocks;
    unsigned int unallocatedInodes;
    unsigned int superblockBlockNumber;
    unsigned int blockSize; //log2 (blocksize) -10 really.
    unsigned int fragmentSize; //log2(fragmentsize) -10 really.
    unsigned int blocksPerBlockGroup;
    unsigned int fragmentsPerBlockGroup;
    unsigned int inodesPerBlockGroup;
    unsigned int lastMountTime; //Posix time.
    unsigned int lastWrittenTime; //Posix time.
    unsigned short totalMountTimesSinceLastCheck;
    unsigned short mountsUntilNextCheck;
    unsigned short ext2Signature;
    unsigned short fileSystemState;
    unsigned short errorAction;
    unsigned short minorVersion;
    unsigned int lastCheck; //Posixt time.
    unsigned int intervalBetweenChecks; //Posix time.
    unsigned int operatingSystemID;
    unsigned int majorVersion;
    unsigned short userIDforReservedBlocks;
    unsigned short groupIDforReservedBlocks;
};

struct BlockGroupDescriptor {
    unsigned int blockBitmapAddress;
    unsigned int inodeBitmapAddress;

    unsigned int inodeTableStart;

    unsigned short unallocatedBlocks;
    unsigned short unallocatedInodes;

    unsigned short directoryCount;

    unsigned char padding[14];
};

#define BLOCK_BUFFER_COUNT 10

struct ext2 {
    struct Superblock* sb;
    struct BlockGroupDescriptor* groupTable;

    unsigned long long firstSector;
    size_t sectorsPerBlock;
    size_t blockSize;
    size_t blockGroupCount;

    /*
     * Cache for indirect pointer blocks.
     */
    unsigned int blockIndexAddress[3];
    unsigned int* blockIndexBuffer[3];

    /*
     * Cache for data blocks.
     * Each entry has an inode owner, and which block it contains.
     *
     * When no entries are available, one is forcibly evicted.
     * The choice is made in a round robin fashion.
     */
    unsigned int evictBlockBuffer;
    unsigned int blockBufferAddress[BLOCK_BUFFER_COUNT];
    unsigned int blockBufferOwner[BLOCK_BUFFER_COUNT];
    void* blockBuffer[BLOCK_BUFFER_COUNT];


    // Intermediate buffer for fragment reads
    unsigned int fragmentReadBlock;
    void* fragmentReadBuffer;

    // Buffer for block group's bitmaps
    unsigned int bitmapBlock;
    unsigned int* bitmapBuffer;
};

int read_sectors(struct ext2* fs, unsigned long long sector, size_t sectors, void* buffer);

int read_block(struct ext2* fs, size_t block, void* buffer);

int read_blocks(struct ext2* fs, size_t start, size_t blocks, void* buffer);

int read_block_fragment(struct ext2* fs, size_t block, size_t offset, size_t len, void* buffer);

int write_sectors(struct ext2* fs, unsigned long long sector, size_t sectors, const void* buffer);

int write_block(struct ext2* fs, size_t block, const void* buffer);

int write_blocks(struct ext2* fs, size_t start, size_t blocks, const void* buffer);

int write_block_fragment(struct ext2* fs, size_t block, size_t offset, size_t len, const void* buffer);

#endif
