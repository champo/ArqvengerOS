#include "system/interrupt.h"
#include "system/common.h"
#include "system/io.h"
#include "system/call.h"

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

void _int20Handler();
void _int21Handler();
void _int80Handler();

void reMapPIC(int offset1,int offset2);

/* This is our IDT with 256 entries */
InterruptDescriptor idt[0x100];

#define PIC1_BASE_OFFSET    0x20
#define PIC2_BASE_OFFSET    0x28

void setupIDT() {
    // We don't actually have to keep this in memory, so it's safe to have it as a local
    InterruptDescriptorTableRegister idtr;

    _cli();
    reMapPIC(PIC1_BASE_OFFSET,PIC2_BASE_OFFSET);
    //TODO: Manage exceptions

    setIdtEntry(idt, 0x80, 0x08, (dword)&_int80Handler, ACS_INT);
    setIdtEntry(idt, 0x20, 0x08, (dword)&_int20Handler, ACS_INT);
    setIdtEntry(idt, 0x21, 0x08, (dword)&_int21Handler, ACS_INT);
    

    idtr.base = (dword) &idt;
    idtr.limit = sizeof(idt) - 1;

    _lidt(&idtr);

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
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

// 8086/88 (MCS-80/85) mode
#define ICW4_8086       0x01        


void reMapPIC(int offset1,int offset2){
    unsigned char mask1,mask2;
    
    // Saving masks
    mask1 = inB(PIC1_DATA);
    mask2 = inB(PIC2_DATA);
    
    // Starting initialization
    outB(PIC1_COMMAND,0x11);
    outB(PIC2_COMMAND,0x11);
    
    // Setting PIC offset
    outB(PIC1_DATA,offset1);
    outB(PIC2_DATA,offset2);
    
    // Continuing initialization
    outB(PIC1_DATA,4);
    outB(PIC2_DATA,2);

    outB(PIC1_DATA,ICW4_8086);
    outB(PIC2_DATA,ICW4_8086);
    
    // Restore masks
    outB(PIC1_DATA,mask1);
    outB(PIC2_DATA,mask2);
}
