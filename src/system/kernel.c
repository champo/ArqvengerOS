#include "system/interrupt.h"
#include "shell/shell.h"
#include "library/stdio.h"
#include "system/call.h"

void kmain(void);

/**
 * Kernel entry point
 */
void kmain(void) {

    FILE files[3];
    for (int i = 0; i < 3; i++) {
        files[i].fd = i;
        files[i].flag = 0;
        files[i].unget = 0;
    }

    stdin = &files[0];
    stdout = &files[1];
    stderr = &files[2];

    // This dandy line, resets the cursor and clears the screen
    _write(1, "\033[1;1H\033[2J", 10);

    setupIDT();
    while (1) {
        shell();
    }
}

