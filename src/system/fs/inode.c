#include "drivers/ext2/inode.h"
#include "system/fs/inode.h"
#include "drivers/ext2/ext2.h"
#include "system/mm.h"
#include "type.h"

static struct ext2* fileSystem = NULL;

struct fs_inode* fs_read_inode(size_t number) {
    struct fs_inode* inode;
    if ((inode = kalloc(sizeof(struct Inode*))) == NULL) {
        return NULL;
    }

    inode->number = number;

    if (fileSystem != NULL && (inode->data = ext2_read_inode(fileSystem, number)) == NULL) {
        return NULL;
        //TODO Free!
    }

    return inode;
}

int fs_read_inode_content(struct fs_inode* inode, size_t offset, size_t size, void* buffer) {
    return ext2_read_inode_content(inode->fileSystem, inode->data, offset, size, buffer);
}

int fs_load(unsigned long long startSector) {
    if ((fileSystem = ext2_load(startSector)) == NULL) {
        return -1;
    }
    return 0;
}
