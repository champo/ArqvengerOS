#include "common.h"
#include "drivers/video.h"
#include "system/interrupt.h"
#include "system/call.h"

/**
 * Kernel entry point
 */
void kmain() {
    clearScreen();
    moveCursor(0, 0);

    setupIDT();

    while (1) {
    }
}


