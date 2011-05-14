#include "system/call.h"
#include "drivers/keyboard.h"

int _ioctl(int fd, int cmd, void* argp) {
    return ioctlKeyboard(cmd, argp);
}
