#ifndef __DRIVERS_TTY__
#define __DRIVERS_TTY__

#include "type.h"
#include "system/process/process.h"

#define NUM_TERMINALS 4

size_t tty_write(const void* buf, size_t length);

void tty_run(char* unused);

size_t tty_read(void* buffer, size_t count);

int ioctlKeyboard(int cmd, void* argp);

void tty_detach_process(struct Process* process);

#endif
