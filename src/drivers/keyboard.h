#ifndef _drivers_keyboard_header
#define _drivers_keyboard_header

#include "type.h"

void readScanCode(void);

size_t readKeyboard(void* buffer, size_t count);

int ioctlKeyboard(int cmd, void* argp);

#endif
