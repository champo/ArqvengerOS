#include "system/call.h"
#include "drivers/tty/tty.h"
#include "system/process/table.h"
#include "system/process/process.h"

size_t _read(int fd, void* buf, size_t length) {
    return readKeyboard(buf, length);
}

size_t _write(int fd, const void* buf, size_t length) {
    return tty_write(buf, length);
}

int _open(const char* path, int flags) {
}

int _creat(const char* path, int mode) {
}

int _close(int fd) {
    struct Process* process = process_table_get(_getpid());

    if (process->fdTable[fd].inode == NULL) {
        return -1;
    }
    if (process->fdTable[fd].ops->close == NULL ||
            process->fdTable[fd].ops->close(&(process->fdTable[fd])) != 0) {
        fs_inode_close(process->fdTable[fd].inode);
        return 1;
    }
    return 0;

}

