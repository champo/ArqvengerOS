#ifndef __DRIVERS_TTY__
#define __DRIVERS_TTY__

#include "type.h"

#define NUM_TERMINALS 4

size_t tty_write(const void* buf, size_t length);

void tty_run(char* unused);

size_t readKeyboard(void* buffer, size_t count);

int ioctlKeyboard(int cmd, void* argp);

#endif
