#include "common.h"
#include "system/interrupt.h"
#include "system/call.h"

/**
 * Kernel entry point
 */
void kmain() {

    // This dandy line, resets the cursor and clears the screen
    write(1, "\033[1;1H\033[2J", 10);

    setupIDT();
    char a[1];
    while (1) {
        read(1, a, 1);
        write(1, "b", 1);
        halt();
    }
}


