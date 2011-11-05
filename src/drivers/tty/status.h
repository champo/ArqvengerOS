#ifndef __DRIVER_TTY_STATUS__
#define __DRIVER_TTY_STATUS__

#include "drivers/videoControl.h"
#include "system/call/ioctl/keyboard.h"

#define CONTROL_BUFFER_LEN 40

#define WAIT_LEN 30

#define BUFFER_ROWS (3 * TOTAL_ROWS)

struct ScreenStatus {
    int cursorPosition;
    int escaped;
    int csi;
    char attribute;
    char controlBuffer[CONTROL_BUFFER_LEN];
    size_t controlBufferPos;
    int visibleLine;
    int lastLine;
    char videoBuffer[2 * LINE_WIDTH * (BUFFER_ROWS + 1)];
    char* writableBuffer;
};

struct Terminal {
    int number;
    int active;
    termios termios;
    struct ScreenStatus screen;
    struct Process* wait[WAIT_LEN];
};

struct Terminal* tty_current(void);

struct Terminal* tty_active(void);

struct Terminal* tty_terminal(int number);

size_t tty_write_active(const char* buf, size_t length);

void tty_screen_init(void);

void tty_screen_change(void);

void tty_change(int active);

void process_scancode(void);

void tty_keyboard_init(void);

void tty_screen_scroll(struct Terminal* term, int line);

#endif
