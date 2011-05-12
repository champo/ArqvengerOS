#include "common.h"
#include "system/interrupt.h"
#include "system/call.h"

/**
 * Kernel entry point
 */
void kmain() {
    setupIDT();

    // This dandy line, resets the cursor and clears the screen
    write(1, "\033[1;1H\033[2J", 10);
    while (1) {}
}


