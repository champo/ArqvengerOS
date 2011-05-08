#include "../include/interrupt.h"
#include "../include/kasm.h"

void setIdtEntry(InterruptDescriptor* table, int entry, byte segmentSelector, dword offset, byte access);

/* This is our IDT with 256 entries */
InterruptDescriptor idt[0x100];

void setupIDT() {
    // We don't actually have to keep this in memory, so it's safe to have it as a local
    InterruptDescriptorTableRegister idtr;

    setIdtEntry(idt, 0x08, 0x08, (dword)&_int08Handler, ACS_INT);
    setIdtEntry(idt, 0x09, 0x08, (dword)&_int09Handler, ACS_INT);

    idtr.base = (dword) &idt;
    idtr.limit = sizeof(idt) - 1;

    _lidt(&idtr);
}

void setIdtEntry(InterruptDescriptor* table, int entry, byte segmentSelector, dword offset, byte access) {
    InterruptDescriptor* item = &table[entry];

    item->selector = segmentSelector;
    item->offset_l = offset & 0xFFFF;
    item->offset_h = (word) (offset >> 16);
    item->access = access;
    item->cero = 0x00;
}

