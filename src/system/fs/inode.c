#include "drivers/ext2/inode.h"
#include "system/fs/inode.h"
#include "system/mm.h"
#include "type.h"

struct Inode* read_inode(struct Filesystem* fs, size_t number) {
    struct Inode* inode;
    if ((inode = kalloc(sizeof(struct Inode*))) == NULL) {
        return NULL;
    }

    inode->number = number;

    if ((inode->data = ext2_read_inode(fs, number)) == NULL) {
        return NULL;
        //TODO Free!
    }

    return inode;
}

int read_inode_content(struct Filesystem* fs, struct Inode* inode, size_t offset, size_t size, void* buffer) {
    return ext2_read_inode_content(fs, inode, offset, size, buffer);
}
