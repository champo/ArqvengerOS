#include "drivers/tty/tty.h"
#include "drivers/tty/status.h"
#include "shell/shell.h"
#include "type.h"
#include "system/process/table.h"
#include "system/scheduler.h"
#include "drivers/ext2/ext2.h"

static int activeTerminal = -1;

static struct Terminal terminals[NUM_TERMINALS];

void tty_run(char* unused) {
    tty_screen_init();
    tty_keyboard_init();

    activeTerminal = 0;
    terminals[activeTerminal].active = 1;

    tty_write("\033[1;1H\033[2J", 10);

    // Spawn the shells (this is a kernel process, so we can do this)
    // TODO: Setup file descriptors
    for (int i = 0; i < NUM_TERMINALS; i++) {
        terminals[i].termios.canon = 1;
        terminals[i].termios.echo = 1;
        process_table_new(shell, NULL, scheduler_current(), 0, i, 1);
    }

    while (1) {
        process_scancode();
    }
}

void tty_change(int active) {

    terminals[activeTerminal].active = 0;
    activeTerminal = active;
    terminals[activeTerminal].active = 1;

    tty_screen_change();
}

struct Terminal* tty_current(void) {

    struct Process* caller = scheduler_current();

    if (caller == NULL || caller->terminal == NO_TERMINAL) {
        return tty_terminal(0);
    } else {
        return tty_terminal(caller->terminal);
    }
}

struct Terminal* tty_active(void) {
    return &terminals[activeTerminal];
}

struct Terminal* tty_terminal(int number) {
    return &terminals[number];
}

