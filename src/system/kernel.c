#include "common.h"
#include "system/interrupt.h"
#include "system/call.h"

/**
 * Kernel entry point
 */
void kmain() {
    setupIDT();

    write(1, "\033[0;0H\033[2J", 10);
    while (1) {}
}


