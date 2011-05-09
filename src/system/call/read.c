#include "system/call.h"
#include "drivers/keyboard.h"

size_t read(int fd, void* buf, size_t length) {
    return readKeyboard(buf, length);
}
