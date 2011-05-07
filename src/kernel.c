#include "../include/kasm.h"
#include "../include/defs.h"
#include "../include/type.h"

DESCR_INT idt[0x100];			/* IDT de 256 entradas*/
IDTR idtr;				/* IDTR */


void int_08() {
}

void setupIDT() {
    setup_IDT_entry(&idt[0x08], 0x08, (dword)&_int_08_hand, ACS_INT);
    setup_IDT_entry(&idt[0x09], 0x08, (dword)&_int09Handler, ACS_INT);

    idtr.base = (dword) &idt;
    idtr.limit = sizeof(idt) - 1;

    _lidt(&idtr);
}

void kmain() {
    k_clear_screen();

    setupIDT();

    _Cli();

    outB(0x21, 0xFC);
    outB(0xA1, 0xFF);

    _Sti();

    while (1) {
        asm volatile ("hlt");
    }
}

