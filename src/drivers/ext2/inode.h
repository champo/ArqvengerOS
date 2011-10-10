#ifndef __EXT2_INODE__
#define __EXT2_INODE__

#include "type.h"
#include "drivers/ext2/internal.h"

#define INODE_TYPE(inode) ((inode->typesAndPermissions & 0xF000) >> 12)

#define INODE_FIFO 0x1
#define INODE_DIR 0x4
#define INODE_FILE 0x8
#define INODE_LINK 0xA

struct ext2_Inode {
    unsigned short typesAndPermissions;
    unsigned short userID;
    unsigned int size;
    unsigned int lastAccess; //Posix time.
    unsigned int creationTime; //Posix time.
    unsigned int lastModification; //Posix time.
    unsigned int deletitionTime; //Posix time.
    unsigned short groupID;
    unsigned short hardLinks;
    unsigned int counDiskSectors;
    unsigned int flags;
    unsigned int operatingSystemValue;
    unsigned int directBlockPointers[12];
    unsigned int singlyIndirectBlockPointer;
    unsigned int doublyIndirectBlockPointer;
    unsigned int triplyIndirectBlockPointer;
    unsigned int generationNumber;
    unsigned int reserverd1;
    unsigned int reserved2;
    unsigned int blockAddressOfFragment;
    unsigned int operatingSystemValue2[3];
};

struct ext2_Inode* ext2_read_inode(struct ext2* fs, size_t number);

int ext2_read_inode_content(struct ext2* fs, struct ext2_Inode* inode, size_t offset, size_t size, void* buffer);

int ext2_write_inode_content(struct ext2* fs, struct ext2_Inode* inode, size_t offset, size_t size, void* buffer);

#endif
