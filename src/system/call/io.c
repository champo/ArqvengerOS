#include "system/call.h"
#include "drivers/tty/tty.h"
#include "system/process/table.h"
#include "system/process/process.h"
#include "system/fs/fs.h"

size_t _read(int fd, void* buf, size_t length) {
    return readKeyboard(buf, length);

    struct Process* process = process_table_get(_getpid());
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL || fileDescriptor->ops->read == NULL) {
        return -1;
    }
    return fileDescriptor->ops->read(fileDescriptor, buf, length);
}

size_t _write(int fd, const void* buf, size_t length) {
    return tty_write(buf, length);

    struct Process* process = process_table_get(_getpid());
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL || fileDescriptor->ops->write == NULL) {
        return -1;
    }
    return fileDescriptor->ops->write(fileDescriptor, buf, length);
}

int _open(const char* path, int flags) {
    /*
     *  Try to open the file, if it fails, create it and then open it :D
     */
}

int _creat(const char* path, int mode) {
}

int _close(int fd) {

    struct Process* process = process_table_get(_getpid());
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL) {
        return -1;
    }
    if (fileDescriptor->ops->close != NULL) {
        fileDescriptor->ops->close(fileDescriptor);
    }
    fs_inode_close(fileDescriptor->inode);
    return 0;

}

