#include "system/call.h"
#include "drivers/tty/tty.h"

size_t _write(int fd, const void* buf, size_t length) {
    return writeScreen(buf, length);
}
