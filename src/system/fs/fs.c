#include "system/fs/fs.h"
#include "drivers/ext2/ext2.h"
#include "system/mm.h"

#define MAX_OPEN_INODES 50

static struct ext2* fs;

static struct fs_Inode *inodeTable[MAX_OPEN_INODES];

// There's only 16 file types supported by ext2
static struct FileDescriptorOps opsTable[16];

static void free_inode(struct fs_Inode* inode);

struct fs_Inode* fs_inode_open(size_t inode) {

    for (size_t i = 0; i < MAX_OPEN_INODES; i++) {
        if (inodeTable[i] != NULL && inodeTable[i]->number == inode) {
            inodeTable[i]->refCount++;
            return inodeTable[i];
        }
    }

    struct fs_Inode* node = ext2_read_inode(fs, inode);
    if (node == NULL) {
        return NULL;
    }

    node->refCount = 1;
    for (size_t i = 0; i < MAX_OPEN_INODES; i++) {
        if (inodeTable[i] == NULL) {
            inodeTable[i] = node;
            return node;
        }
    }

    free_inode(node);

    return NULL;
}

void free_inode(struct fs_Inode* inode) {
    kfree(inode->data);
    kfree(inode);
}

void fs_inode_close(struct fs_Inode* inode) {

    inode->refCount--;
    if (inode->refCount == 0) {

        for (size_t i = 0; i < MAX_OPEN_INODES; i++) {
            if (inodeTable[i] != NULL && inodeTable[i] == inode) {
                free_inode(inode);
                inodeTable[i] = NULL;
            }
        }
    }
}

struct fs_Inode* fs_root(void) {
    return fs_inode_open(2);
}

int fs_load(void) {
    fs = ext2_load(0);
    fs_inode_open(2);

    return 0;
}

void fs_register_ops(int fileType, struct FileDescriptorOps ops) {
    opsTable[fileType] = ops;
}

struct FileDescriptor fs_fd(struct fs_Inode* inode, int flags) {
    return (struct FileDescriptor) {
        .inode = inode,
        .offset = 0,
        .flags = flags,
        .ops = &opsTable[INODE_TYPE(inode->data)]
    };
}

