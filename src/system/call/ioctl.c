#include "system/call.h"
#include "drivers/keyboard.h"

int ioctl(int fd, int cmd, void* argp) {
    return ioctlKeyboard(cmd, argp);
}
