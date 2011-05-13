#include "system/interrupt.h"
#include "system/common.h"
#include "system/io.h"

/* Flags para derechos de acceso de los segmentos */
#define ACS_PRESENT     0x80            /* segmento presente en memoria */
#define ACS_CSEG        0x18            /* segmento de codigo */
#define ACS_DSEG        0x10            /* segmento de datos */
#define ACS_READ        0x02            /* segmento de lectura */
#define ACS_WRITE       0x02            /* segmento de escritura */
#define ACS_IDT         ACS_DSEG
#define ACS_INT_386 	0x0E		/* Interrupt GATE 32 bits */
#define ACS_INT         ( ACS_PRESENT | ACS_INT_386 )


#define ACS_CODE        (ACS_PRESENT | ACS_CSEG | ACS_READ)
#define ACS_DATA        (ACS_PRESENT | ACS_DSEG | ACS_WRITE)
#define ACS_STACK       (ACS_PRESENT | ACS_DSEG | ACS_WRITE)

/* Tell the compiler to pack the following structs to 1 byte (No 4 byte padding) */
#pragma pack (1)

/* Segment Descriptor */
typedef struct {
    word limit;
    word base_l;
    byte base_m;
    byte access;
    byte attribs;
    byte base_h;
} SegmentDescriptor;


/* Interrupt descriptor */
typedef struct {
    word offset_l;
    word selector;
    byte cero;
    byte access;
    word offset_h;
} InterruptDescriptor;

/* Interrupt Descriptor Table Register */
typedef struct {
    word limit;
    dword base;
} InterruptDescriptorTableRegister;

void setIdtEntry(InterruptDescriptor* table, int entry, byte segmentSelector, dword offset, byte access);

/* These are all ASM functions used only here, so we just keep the def here */
void _lidt(InterruptDescriptorTableRegister* idtr);
void _cli(void);
void _sti(void);

void _int08Handler();
void _int09Handler();
void _int80Handler();

/* This is our IDT with 256 entries */
InterruptDescriptor idt[0x100];

void setupIDT() {
    // We don't actually have to keep this in memory, so it's safe to have it as a local
    InterruptDescriptorTableRegister idtr;

    //TODO: Remap the PIC
    //TODO: Manage exceptions


    setIdtEntry(idt, 0x80, 0x08, (dword)&_int80Handler, ACS_INT);

    setIdtEntry(idt, 0x08, 0x08, (dword)&_int08Handler, ACS_INT);
    setIdtEntry(idt, 0x09, 0x08, (dword)&_int09Handler, ACS_INT);
    
    idtr.base = (dword) &idt;
    idtr.limit = sizeof(idt) - 1;

    _lidt(&idtr);

    _cli();

    setInterruptHandlerTable();

    /* Enable the interrupts we need in the PIC */
    outB(0x21,0xFC);
    outB(0xA1,0xFF);

    _sti();
}

void setIdtEntry(InterruptDescriptor* table, int entry, byte segmentSelector, dword offset, byte access) {
    InterruptDescriptor* item = &table[entry];

    item->selector = segmentSelector;
    item->offset_l = offset & 0xFFFF;
    item->offset_h = (word) (offset >> 16);
    item->access = access;
    item->cero = 0x00;
}

