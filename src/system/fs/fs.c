#include "system/fs/fs.h"
#include "system/mm/allocator.h"
#include "system/kprintf.h"
#include "system/process/process.h"
#include "drivers/ext2/ext2.h"
#include "drivers/ext2/directory.h"
#include "library/string.h"

#define MAX_OPEN_INODES 50

static struct ext2* fs;

static struct fs_Inode *inodeTable[MAX_OPEN_INODES];

// There's only 16 file types supported by ext2
static struct FileDescriptorOps opsTable[16];

static void free_inode(struct fs_Inode* inode);

static int remove_link(struct fs_Inode* path, const char* name, struct fs_Inode* inode);

static int add_link(struct fs_Inode* path, const char* name, struct fs_Inode* inode);

struct fs_Inode* fs_inode_open(size_t inode) {

    for (size_t i = 0; i < MAX_OPEN_INODES; i++) {
        if (inodeTable[i]) {
            if (inodeTable[i]->refCount <= 0) {
                kprintf("Inode %u in entry %u is broken.\n", inodeTable[i]->number, i);
                while (1);
            }
        }
    }
    for (size_t i = 0; i < MAX_OPEN_INODES; i++) {
        if (inodeTable[i] != NULL && inodeTable[i]->number == inode) {
            inodeTable[i]->refCount++;
            return inodeTable[i];
        }
    }

    //TODO: I need locking to avoid duplicate inodes
    struct fs_Inode* node = ext2_read_inode(fs, inode);
    node->extra = NULL;
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
            if (inodeTable[i] == inode) {
                free_inode(inode);
                inodeTable[i] = NULL;
                return;
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

void fs_fd(struct FileDescriptor* to, struct fs_Inode* inode, int flags) {
    inode->refCount++;

    to->inode = inode;
    to->offset = 0;
    to->flags = flags;
    to->ops = &opsTable[INODE_TYPE(inode->data)];

    if (to->ops->open) {
        to->ops->open(to);
    }
}

void fs_dup(struct FileDescriptor* to, struct FileDescriptor fd) {
    fs_fd(to, fd.inode, fd.flags);
}

struct fs_DirectoryEntry fs_findentry(struct fs_Inode* path, const char* name) {
    struct DirectoryEntry entry = ext2_dir_find(path, name);
    struct fs_DirectoryEntry res;

    if (entry.entryLength) {
        res.inode = entry.inode;
        res.length = entry.nameLength;
        strcpy(res.name, entry.name);
    } else {
        res.inode = 0;
    }

    return res;
}

int fs_rmdir(struct fs_Inode* path, const char* name) {

    struct fs_DirectoryEntry entry = fs_findentry(path, name);
    if (!entry.inode) {
        return ENOENT;
    }

    struct fs_Inode* dir = fs_inode_open(entry.inode);
    if (dir == NULL) {
        return EIO;
    }

    if (INODE_TYPE(dir->data) != INODE_DIR) {
        fs_inode_close(dir);
        return ENOTDIR;
    }

    size_t offset = 0;
    struct DirectoryEntry child = ext2_dir_read(dir, offset);
    while (child.entryLength != 0) {
        if (child.inode != path->number && child.inode != entry.inode) {
            fs_inode_close(dir);
            return ENOTEMPTY;
        }

        offset += child.entryLength;
        child = ext2_dir_read(dir, offset);
    }

    int res = remove_link(path, name, dir);
    if (dir->data->hardLinks == 1) {
        remove_link(dir, ".", dir);
        remove_link(dir, "..", path);
    }
    fs_inode_close(dir);

    return res;
}

int fs_unlink(struct fs_Inode* path, const char* name) {

    struct fs_DirectoryEntry entry = fs_findentry(path, name);
    if (!entry.inode) {
        return ENOENT;
    }

    struct fs_Inode* inode = fs_inode_open(entry.inode);
    if (inode == NULL) {
        return EIO;
    }
    if (INODE_TYPE(inode->data) == INODE_DIR) {
        fs_inode_close(inode);
        return EISDIR;
    }

    int res = remove_link(path, name, inode);
    fs_inode_close(inode);

    return res;
}

int fs_symlink(struct fs_Inode* path, const char* entry, const char* to) {

    if (fs_mknod(path, entry, INODE_LINK) != 0) {
        return -1;
    }

    struct fs_DirectoryEntry direntry = fs_findentry(path, entry);

    if (direntry.inode == 0) {
        //TODO should the new node be unlinked?
        return -1;
    }

    struct fs_Inode* file = fs_inode_open(direntry.inode);

    fs_set_permission(file, 00666);

    int length = strlen(to);

    if (length != ext2_write_inode_content(file, 0, length, to)) {
        //TODO hay que hacerle unlink al mknod?
        fs_inode_close(file);
        return -1;
    }
    fs_inode_close(file);
    return 0;
}

char* fs_symlink_read(struct fs_Inode* symlink, int size, char* buff) {

    if (size != ext2_read_inode_content(symlink, 0, size, buff)) {
        return NULL;
    }
    buff[size] = '\0';
    return buff;

}

int fs_rename(struct fs_Inode* from, const char* original, struct fs_Inode* to, const char* new) {

    struct fs_DirectoryEntry entry = fs_findentry(from, original);
    if (!entry.inode) {
        return ENOENT;
    }

    struct fs_Inode* inode = fs_inode_open(entry.inode);

    if (inode == NULL) {
        return ENOENT;
    }
    if (INODE_TYPE(inode->data) == INODE_DIR) {
        if (remove_link(inode, "..", from) == -1) {
            fs_inode_close(inode);
            return EIO;
        }

        if (add_link(inode, "..", to) == -1) {
            fs_inode_close(inode);
            return EIO;
        }
    }
    fs_inode_close(inode);

    if (ext2_dir_add(to, new, entry.inode) == -1) {
        return EIO;
    }

    if (ext2_dir_remove(from, original) == -1) {
        return EIO;
    }



    return 0;
}

int fs_permission(struct fs_Inode* inode) {
    return INODE_PERMISSIONS(inode->data);
}

int fs_set_permission(struct fs_Inode* inode, int perm) {
    inode->data->typesAndPermissions = (INODE_TYPE(inode->data) << 12) | perm;
    return ext2_write_inode(inode) == -1 ? EIO : 0;
}

int fs_mknod(struct fs_Inode* path, const char* name, int type) {

    if (fs_findentry(path, name).inode != 0) {
        return EEXIST;
    }

    struct Process* process = scheduler_current();

    struct fs_Inode* nod = ext2_create_inode(fs, type, PERM_DEFAULT, process->uid, process->gid);
    if (NULL == nod) {
        return EIO;
    }

    int res = add_link(path, name, nod);
    free_inode(nod);

    return res;
}

int fs_mkdir(struct fs_Inode* path, const char* name) {

    if (fs_findentry(path, name).inode != 0) {
        return EEXIST;
    }

    struct Process* process = scheduler_current();

    struct fs_Inode* dir = ext2_dir_create(fs, PERM_DEFAULT, process->uid, process->gid);

    int res = add_link(path, name, dir);
    if (res == 0) {
        add_link(dir, ".", dir);
        add_link(dir, "..", path);
    }

    free_inode(dir);

    return res;
}

int remove_link(struct fs_Inode* path, const char* name, struct fs_Inode* inode) {

    int res;

    if (ext2_dir_remove(path, name) == -1) {
        return EIO;
    }

    inode->data->hardLinks--;
    if (inode->data->hardLinks == 0) {
        res = ext2_delete_inode(inode);
    } else {
        res = ext2_write_inode(inode);
    }

    return res == -1 ? EIO : 0;
}

int add_link(struct fs_Inode* path, const char* name, struct fs_Inode* inode) {

    if (ext2_dir_find(path, name).entryLength != 0) {
        return EEXIST;
    }

    if (ext2_dir_add(path, name, inode->number) == -1) {
        return EIO;
    }

    inode->data->hardLinks++;
    return ext2_write_inode(inode);
}

int fs_get_inode_size(struct fs_Inode* inode) {
    return inode->data->size;
}

int fs_fd_close(struct FileDescriptor* fd) {

    int res = 0;
    if (fd->ops->close != NULL) {
        res = fd->ops->close(fd);
    }
    fs_inode_close(fd->inode);
    fd->inode = NULL;

    return res;
}

/**
 * Changes the uid and gid of a file.
 *
 * @param inode, the inode representing the file.
 * @param uid, the new user id.
 * @param gid, the new group id.
 * @return 0.
 */
int fs_set_own(struct fs_Inode* inode, int uid, int gid) {

    inode->data->uid = uid;
    inode->data->gid = gid;

    return ext2_write_inode(inode) == -1 ? EIO : 0;
}

