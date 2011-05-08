#include "common.h"
#include "drivers/video.h"
#include "drivers/keyboard.h"
#include "system/interrupt.h"

/**
 * Kernel entry point
 */
void kmain() {
    clearScreen();
    moveCursor(0, 0);

    setupIDT();

    char a;
    while (1) {
        read(0, &a, 1);
        write(&a, 1);
    }
}


