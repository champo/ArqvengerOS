#ifndef __DRIVERS_TTY__
#define __DRIVERS_TTY__

#include "type.h"

size_t writeScreen(const void* buf, size_t length);

void initScreen(void);

size_t readKeyboard(void* buffer, size_t count);

void process_scancode(void);

int ioctlKeyboard(int cmd, void* argp);

void initKeyboard(void);

#endif
