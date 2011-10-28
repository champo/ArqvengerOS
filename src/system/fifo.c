#include "system/fifo.h"
#include "system/fs/fs.h"
#include "system/processQueue.h"
#include "constants.h"
#include "system/process/table.h"
#include "system/scheduler.h"
#include "library/string.h"

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

static void fifo_open(struct FileDescriptor* fd);

static void wake_all(struct ProcessQueue* queue);

static void wait(struct ProcessQueue* queue, struct Process* process);

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

void fifo_open(struct FileDescriptor* fd) {

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
        if (fifo->writers == 0) {
            wait(&fifo->readWait, scheduler_current());
        }
    } else if (modeFlags == O_WRONLY) {

        fifo->writers++;
        if (fifo->readers == 0) {
            wait(&fifo->writeWait, scheduler_current());
        }
    } else {
        fifo->readers++;
        fifo->writers++;
    }

    wake_all(&fifo->writeWait);
    wake_all(&fifo->readWait);
}

size_t fifo_write(struct FileDescriptor* fd, const void* buffer, size_t len) {

    struct FIFO* fifo = fd->inode->extra;
    kprintf("FIRST");
    while (fifo->size - fifo->used < len) {

        if (fifo->readers == 0) {
            scheduler_current()->schedule.ioWait = 0;
            kprintf("WTF");
            return 0;
        }

        kprintf("WTT");
        wait(&fifo->writeWait, scheduler_current());
    }

    scheduler_current()->schedule.ioWait = 0;
    kprintf("HeY");
    if (fifo->readers == 0) {
        kprintf("Hey");
        return 0;
    }

    char* buf = (char*) fifo->buffer + fifo->used;
    memcpy(buf, buffer, len);
    fifo->used += len;

    kprintf("before");
    wake_all(&fifo->readWait);
    kprintf("afteR");

    return len;
}

size_t fifo_read(struct FileDescriptor* fd, void* buffer, size_t len) {

    struct FIFO* fifo = fd->inode->extra;
    while (fifo->used == 0) {

        if (fifo->writers == 0) {
            scheduler_current()->schedule.ioWait = 0;
            return 0;
        }

        wait(&fifo->readWait, scheduler_current());
    }

    scheduler_current()->schedule.ioWait = 0;
    if (fifo->writers == 0) {
        return 0;
    }

    size_t length = len;
    if (length > fifo->used) {
        length = fifo->used;
    }

    memcpy(buffer, fifo->buffer, length);
    char* buf = fifo->buffer;
    for (size_t i = length; i < fifo->used; i++) {
        buf[i - length] = buf[i];
    }
    fifo->used -= length;

    wake_all(&fifo->writeWait);

    return length;
}

int fifo_close(struct FileDescriptor* fd) {

    if (fd->inode->extra == NULL) {
        kprintf("This be wrong.\n");
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

    kprintf("%u %u\n", fifo->readers, fifo->writers);
    if ((fifo->readers | fifo->writers) == 0) {
        kprintf("Closing");
        // We be dead!
        kfree(fifo->buffer);
        kfree(fifo);

        fd->inode->extra = NULL;
    } else {
        // Removing one process from the FIFO requires that all involved
        // process are woken up. This might be the last reader, or last writer.
        // Also, it avoids having to ask for any process specific data when doing this.

        kprintf("WAKE!");
        wake_all(&fifo->writeWait);
        wake_all(&fifo->readWait);
    }

    return 0;
}

void wake_all(struct ProcessQueue* queue) {
    struct Process* process = process_queue_pop(queue);

    while (process != NULL) {
        process_table_unblock(process);
        process = process_queue_pop(queue);
    }
}

void wait(struct ProcessQueue* queue, struct Process* process) {

    process_queue_push(queue, process);

    process->schedule.ioWait = 1;
    process_table_block(process);

    scheduler_do();
}

