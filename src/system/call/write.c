#include "system/call.h"
#include "drivers/tty/tty.h"
#include "system/scheduler.h"

size_t _write(int fd, const void* buf, size_t length) {
    return tty_write(scheduler_current()->terminal, buf, length);
}
