#include "system/call.h"
#include "drivers/tty/tty.h"

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
}

