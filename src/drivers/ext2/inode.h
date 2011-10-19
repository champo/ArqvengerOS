#ifndef __EXT2_INODE__
#define __EXT2_INODE__

#include "type.h"
#include "drivers/ext2/internal.h"
#include "system/fs/inode.h"

#define INODE_TYPE(inode) (((inode)->typesAndPermissions & 0xF000) >> 12)
#define INODE_PERMISSIONS(inode) ((inode)->typesAndPermissions & 0x0FFF)

#define INODE_FIFO 0x1
#define INODE_DIR 0x4
#define INODE_FILE 0x8
#define INODE_LINK 0xA

struct ext2_Inode {
    unsigned short typesAndPermissions;
    unsigned short uid;
    unsigned int size;
    unsigned int lastAccess; //Posix time.
    unsigned int creationTime; //Posix time.
    unsigned int lastModification; //Posix time.
    unsigned int deletionTime; //Posix time.
    unsigned short gid;
    unsigned short hardLinks;
    unsigned int countDiskSectors;
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

/**
 * Read an inode given its number.
 *
 * @param fs The current fs.
 * @param number The inode number.
 *
 * @return The inode on success, NULL on error.
 */
struct fs_Inode* ext2_read_inode(struct ext2* fs, size_t number);

/**
 * Read content off inode.
 *
 * @param inode The inode to read off.
 * @param offset The start of the data to read.
 * @param size The number of bytes of data to read.
 * @param buffer The buffer to write to.
 *
 * @return  the number of read bytes, -1 on error
 */
int ext2_read_inode_content(struct fs_Inode* inode, size_t offset, size_t size, void* buffer);

/**
 * Write content to an inode.
 *
 * @param inode The inode to write to.
 * @param offset The first byte to write to.
 * @param size The number of bytes of data to write.
 * @param buffer The buffer to read from.
 *
 * @return the number of written bytes, -1 on error
 */
int ext2_write_inode_content(struct fs_Inode* inode, size_t offset, size_t size, void* buffer);

/**
 * Write an inode to the inode table.
 *
 * @param inode The inode to write.
 *
 * @return 0 on success, -1 on error
 */
int ext2_write_inode(struct fs_Inode* inode);

/**
 * Create a new inode.
 *
 * @param fs The current fs.
 * @param type The inode type.
 * @param permissions The permissions to set.
 * @param uid Owner id.
 * @param gid Group id.
 *
 * @return The inode on success, or NULL on error
 */
struct fs_Inode* ext2_create_inode(struct ext2* fs, int type, int permissions, int uid, int gid);

/**
 * Delete an inode, and free all associated resources.
 *
 * @param inode The inode to free.
 *
 * @return 0 on success, -1 on error
 */
int ext2_delete_inode(struct fs_Inode* inode);

#endif
