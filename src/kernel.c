#include "kasm.h"
#include "type.h"
#include "video.h"
#include "interrupt.h"

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

    char a;
    while (1) {
        read(0, &a, 1);
        write(&a, 1);
        asm volatile ("hlt");
    }
}


