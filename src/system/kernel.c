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

    char a[1];
    while (1) {
        read(0, (void*) a, 1);
        write(0, (void*) a, 1);
    }
}


