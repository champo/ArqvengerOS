#ifndef __DRIVERS_TTY__
#define __DRIVERS_TTY__

#include "type.h"

#define NUM_TERMINALS 4

size_t tty_write(int terminal, const void* buf, size_t length);

void tty_run(char* unused);

void tty_screen_init(void);

void tty_screen_change(int terminal);

void tty_change(int active);

size_t readKeyboard(void* buffer, size_t count);

void process_scancode(void);

int ioctlKeyboard(int cmd, void* argp);

void tty_keyboard_init(void);

#endif
