#include "system/interrupt.h"
#include "system/call.h"
#include "shell/shell.h"

/**
 * Kernel entry point
 */
void kmain() {

    // This dandy line, resets the cursor and clears the screen
    write(1, "\033[1;1H\033[2J", 10);

    setupIDT();
    while (1) {
        shell();
    }
}


