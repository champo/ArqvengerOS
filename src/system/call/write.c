#include "system/call.h"
#include "drivers/video.h"

size_t write(int fd, const void* buf, size_t length) {
    return writeScreen(buf, length);
}
