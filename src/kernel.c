#include "../include/kasm.h"
#include "../include/type.h"
#include "../include/video.h"
#include "../include/interrupt.h"

void int08() {
}

/**
 * Kernel entry point
 */
void kmain() {
    clearScreen();
    moveCursor(0, 0);

    setupIDT();

    _cli();

    /* Enable the interrupts we need in the PIC */
    outB(0x21,0xFC);
    outB(0xA1,0xFF);

    _sti();

    while (1) {
        asm volatile ("hlt");
    }
}


