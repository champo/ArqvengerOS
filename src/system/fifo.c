#include "system/fifo.h"
#include "system/fs/fs.h"
#include "system/processQueue.h"
#include "constants.h"

#define FIFO_BUFFER_SIZE 1024

struct FIFO {
    size_t readers;
    size_t writers;

    struct ProcessQueue readWait;
    struct ProcessQueue writeWait;

    void* buffer;
    size_t used;
    size_t size;
};

static size_t fifo_write(struct FileDescriptor* fd, const void* buffer, size_t len);

static size_t fifo_read(struct FileDescriptor* fd, void* buffer, size_t len);

static int fifo_close(struct FileDescriptor* fd);

static int fifo_open(struct FileDescriptor* fd);

void fifo_init(void) {
    fs_register_ops(INODE_FIFO, (struct FileDescriptorOps) {
            .open = fifo_open,
            .readdir = NULL,
            .ioctl = NULL,
            .write = fifo_write,
            .read = fifo_read,
            .close = fifo_close
    });
}

int fifo_open(struct FileDescriptor* fd) {

    struct FIFO* fifo;
    if (fd->inode->extra) {
        fifo = fd->inode->extra;
    } else {
        fifo = kalloc(sizeof(struct FIFO));

        fifo->size = FIFO_BUFFER_SIZE;
        fifo->used = 0;
        fifo->buffer = kalloc(fifo->size);

        fifo->readers = 0;
        fifo->readWait.first = NULL;
        fifo->readWait.last = NULL;

        fifo->writers = 0;
        fifo->writeWait.first = NULL;
        fifo->writeWait.last = NULL;

        fd->inode->extra = fifo;
    }

    int modeFlags = fd->flags & 0x3;
    if (modeFlags == O_RDONLY) {
        fifo->readers++;
    } else if (modeFlags == O_WRONLY) {
        fifo->writers++;
    } else {
        fifo->readers++;
        fifo->writers++;
    }
}

size_t fifo_write(struct FileDescriptor* fd, const void* buffer, size_t len) {
}

size_t fifo_read(struct FileDescriptor* fd, void* buffer, size_t len) {
}

int fifo_close(struct FileDescriptor* fd) {

    if (fd->inode->extra == NULL) {
        return 0;
    }

    struct FIFO* fifo = fd->inode->extra;
    int modeFlags = fd->flags & 0x3;
    if (modeFlags == O_RDONLY) {
        fifo->readers--;
    } else if (modeFlags == O_WRONLY) {
        fifo->writers--;
    } else {
        fifo->readers--;
        fifo->writers--;
    }

    if ((fifo->readers | fifo->writers) == 0) {
        // We be dead!
        kfree(fifo->buffer);
        kfree(fifo);

        fd->inode->extra = NULL;
    } else {
        // Removing one process from the FIFO requires that all involved
        // process are woken up. This might be the last reader, or last writer.
        // Also, it avoids having to ask for any process specific data when doing this.

        //TODO: Wake everyone the hell up :D
    }

    return 0;
}

