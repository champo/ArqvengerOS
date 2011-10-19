#include "drivers/ext2/ext2.h"
#include "drivers/ext2/internal.h"
#include "drivers/ext2/superblock.h"
#include "drivers/ext2/blockGroup.h"
#include "drivers/ext2/inode.h"
#include "drivers/ext2/directory.h"
#include "system/fs/fs.h"
#include "system/mm.h"

static size_t inode_write(struct FileDescriptor* fd, const void* buffer, size_t len);

static size_t inode_read(struct FileDescriptor* fd, void* buffer, size_t len);

static struct fs_DirectoryEntry readdir(struct FileDescriptor* fd);

struct ext2* ext2_load(unsigned long long startSector) {

    struct ext2* fs = kalloc(sizeof(struct ext2));

    fs->firstSector = startSector;

    ext2_superblock_init(fs);

    fs->blockSize = 1024 << fs->sb->blockSize;
    fs->sectorsPerBlock = fs->blockSize / SECTOR_SIZE;
    fs->blockGroupCount = ext2_get_total_block_groups(fs->sb);

    // Setup the various caches

    for (int i = 0; i < 3; i++) {
        fs->blockIndexAddress[i] = 0;
        fs->blockIndexBuffer[i] = kalloc(fs->blockSize);
    }

    fs->fragmentReadBlock = 0;
    fs->fragmentReadBuffer = kalloc(fs->blockSize);

    fs->evictBlockBuffer = 0;
    for (int i = 0; i < BLOCK_BUFFER_COUNT; i++) {
        fs->blockBufferAddress[i] = 0;
        fs->blockBufferOwner[i] = NULL;
        fs->blockBuffer[i] = kalloc(fs->blockSize);
    }

    fs->bitmapBlock = 0;
    fs->bitmapBuffer = kalloc(fs->blockSize);

    ext2_read_blockgroup_table(fs);

    fs_register_ops(INODE_DIR, (struct FileDescriptorOps) {
            .write = NULL,
            .read = NULL,
            .ioctl = NULL,
            .readdir = readdir,
            .close = NULL
    });

    struct FileDescriptorOps fileOps = {
        .write = inode_write,
        .read = inode_read,
        .ioctl = NULL,
        .readdir = NULL,
        .close = NULL
    };

    fs_register_ops(INODE_FILE, fileOps);
    fs_register_ops(INODE_LINK, fileOps);

    return fs;
}

size_t inode_write(struct FileDescriptor* fd, const void* buffer, size_t len) {

    int res = ext2_write_inode_content(fd->inode, fd->offset, len, buffer);
    if (res == -1) {
        return -1;
    } else {
        fd->offset += res;
        return res;
    }
}

size_t inode_read(struct FileDescriptor* fd, void* buffer, size_t len) {
    int res = ext2_read_inode_content(fd->inode, fd->offset, len, buffer);
    if (res == -1) {
        return -1;
    } else {
        fd->offset += res;
        return res;
    }
}

struct fs_DirectoryEntry readdir(struct FileDescriptor* fd) {
    struct DirectoryEntry entry;
    do {
        entry = ext2_dir_read(fd->inode, fd->offset);
        fd->offset += entry.entryLength;
    } while (entry.inode == 0);

    if (entry.entryLength == 0) {
        return (struct fs_DirectoryEntry) {
            .inode = 0,
            .length = 0
        };
    }

    struct fs_DirectoryEntry res = {
        .inode = entry.inode,
        .length = entry.nameLength
    };

    strcpy(res.name, entry.name);

    return res;
}

