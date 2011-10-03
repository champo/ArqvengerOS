#include "drivers/tty/tty.h"
#include "shell/shell.h"
#include "type.h"
#include "system/process/table.h"

static int activeTerminal = -1;

void tty_run(char* unused) {
    tty_screen_init();
    tty_keyboard_init();

    // Spawn the shells (this is a kernel process, so we can do this)
    // TODO: Setup file descriptors
    for (int i = 0; i < NUM_TERMINALS; i++) {
        process_table_new(shell, NULL, scheduler_current(), 0, i, 1);
    }

    activeTerminal = 0;

    tty_write(0, "\033[1;1H\033[2J", 10);
}

void tty_change(int active) {
    activeTerminal = active;
    tty_screen_change(active);
}

