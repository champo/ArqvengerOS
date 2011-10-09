#ifndef __EXT2_INODE__
#define __EXT2_INODE__

#include "type.h"

struct ext2_Inode {
    word typesAndPermissions;
    word userID;
    dword lowerSize;
    dword lastAccess; //Posix time.
    dword creationTime; //Posix time.
    dword lastModification; //Posix time.
    dword deletitionTime; //Posix time.
    word groupID;
    word hardLinks;
    dword counDiskSectors;
    dword flags;
    dword operatingSystemValue;
    dword directBlockPointers[12];
    dword singlyIndirectBlockPointer;
    dword doublyIndirectBlockPointer;
    dword triplyIndirectBlockPointer;
    dword generationNumber;
    dword reserverd1;
    dword reserved2;
    dword blockAddressOfFragment;
    dword operatingSystemValue2[3];
};

struct ext2_Inode* ext2_read_inode(struct ext2* fs, int number);

#endif
