#include "system/interrupt.h"
#include "system/common.h"
#include "system/io.h"
#include "system/call.h"
#include "system/interrupt/handler.h"
#include "system/call/codes.h"

/* Flags para derechos de acceso de los segmentos */
#define ACS_PRESENT         0x80            /* segmento presente en memoria */
#define ACS_CSEG            0x18            /* segmento de codigo */
#define ACS_DSEG            0x10            /* segmento de datos */
#define ACS_READ            0x02            /* segmento de lectura */
#define ACS_WRITE           0x02            /* segmento de escritura */
#define ACS_IDT             ACS_DSEG
#define ACS_INT_386 	    0x0E		/* Interrupt GATE 32 bits */
#define ACS_INT             (ACS_PRESENT | ACS_INT_386 )


#define ACS_CODE            (ACS_PRESENT | ACS_CSEG | ACS_READ)
#define ACS_DATA            (ACS_PRESENT | ACS_DSEG | ACS_WRITE)
#define ACS_STACK           (ACS_PRESENT | ACS_DSEG | ACS_WRITE)

#define PIC1_BASE_OFFSET    0x20
#define PIC2_BASE_OFFSET    0x28

#define PIC1_COMMAND        0x20
#define PIC1_DATA           0x21
#define PIC2_COMMAND        0xA0
#define PIC2_DATA           0xA1

// 8086/88 (MCS-80/85) mode
#define ICW4_8086           0x01

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

/* This is our IDT with 256 entries */
static InterruptDescriptor idt[0x100];

static void setIdtEntry(InterruptDescriptor* table, int entry, byte segmentSelector, dword offset, byte access);
static void reMapPIC(int offset1,int offset2);

/* These are all ASM functions used only here, so we just keep the def here */
void _lidt(InterruptDescriptorTableRegister* idtr);
void _cli(void);
void _sti(void);

void _int00Handler(void);
void _int01Handler(void);
void _int02Handler(void);
void _int03Handler(void);
void _int04Handler(void);
void _int05Handler(void);
void _int06Handler(void);
void _int07Handler(void);
void _int08Handler(void);
void _int09Handler(void);
void _int0AHandler(void);
void _int0BHandler(void);
void _int0CHandler(void);
void _int0DHandler(void);
void _int0EHandler(void);
void _int0FHandler(void);
void _int10Handler(void);
void _int11Handler(void);
void _int12Handler(void);
void _int13Handler(void);
void _int14Handler(void);
void _int15Handler(void);
void _int16Handler(void);
void _int17Handler(void);
void _int18Handler(void);
void _int19Handler(void);
void _int1AHandler(void);
void _int1BHandler(void);
void _int1CHandler(void);
void _int1DHandler(void);
void _int1EHandler(void);
void _int1FHandler(void);

void _int20Handler(void);
void _int21Handler(void);
void _int80Handler(void);



/**
 * Initializes the Interrupt Descriptor Table.
 *
 * Remaps the PIC, and loads the interrput and exceptions handlers.
 */
void setupIDT(void) {
    // We don't actually have to keep this in memory, so it's safe to have it as a local.
    InterruptDescriptorTableRegister idtr;

    // Disabling interrupts to make sure we're in absolute control.
    _cli();
    reMapPIC(PIC1_BASE_OFFSET,PIC2_BASE_OFFSET);

    setIdtEntry(idt, 0x80, 0x08, (dword)&_int80Handler, ACS_INT);
    setIdtEntry(idt, 0x20, 0x08, (dword)&_int20Handler, ACS_INT);
    setIdtEntry(idt, 0x21, 0x08, (dword)&_int21Handler, ACS_INT);

    setIdtEntry(idt, 0x00, 0x08, (dword)&_int00Handler, ACS_INT);
    setIdtEntry(idt, 0x01, 0x08, (dword)&_int01Handler, ACS_INT);
    setIdtEntry(idt, 0x02, 0x08, (dword)&_int02Handler, ACS_INT);
    setIdtEntry(idt, 0x03, 0x08, (dword)&_int03Handler, ACS_INT);
    setIdtEntry(idt, 0x04, 0x08, (dword)&_int04Handler, ACS_INT);
    setIdtEntry(idt, 0x05, 0x08, (dword)&_int05Handler, ACS_INT);
    setIdtEntry(idt, 0x06, 0x08, (dword)&_int06Handler, ACS_INT);
    setIdtEntry(idt, 0x07, 0x08, (dword)&_int07Handler, ACS_INT);
    setIdtEntry(idt, 0x08, 0x08, (dword)&_int08Handler, ACS_INT);
    setIdtEntry(idt, 0x09, 0x08, (dword)&_int09Handler, ACS_INT);
    setIdtEntry(idt, 0x0A, 0x08, (dword)&_int0AHandler, ACS_INT);
    setIdtEntry(idt, 0x0B, 0x08, (dword)&_int0BHandler, ACS_INT);
    setIdtEntry(idt, 0x0C, 0x08, (dword)&_int0CHandler, ACS_INT);
    setIdtEntry(idt, 0x0D, 0x08, (dword)&_int0DHandler, ACS_INT);
    setIdtEntry(idt, 0x0E, 0x08, (dword)&_int0EHandler, ACS_INT);
    setIdtEntry(idt, 0x0F, 0x08, (dword)&_int0FHandler, ACS_INT);
    setIdtEntry(idt, 0x10, 0x08, (dword)&_int10Handler, ACS_INT);
    setIdtEntry(idt, 0x11, 0x08, (dword)&_int11Handler, ACS_INT);
    setIdtEntry(idt, 0x12, 0x08, (dword)&_int12Handler, ACS_INT);
    setIdtEntry(idt, 0x13, 0x08, (dword)&_int13Handler, ACS_INT);
    setIdtEntry(idt, 0x14, 0x08, (dword)&_int14Handler, ACS_INT);
    setIdtEntry(idt, 0x15, 0x08, (dword)&_int15Handler, ACS_INT);
    setIdtEntry(idt, 0x16, 0x08, (dword)&_int16Handler, ACS_INT);
    setIdtEntry(idt, 0x17, 0x08, (dword)&_int17Handler, ACS_INT);
    setIdtEntry(idt, 0x18, 0x08, (dword)&_int18Handler, ACS_INT);
    setIdtEntry(idt, 0x19, 0x08, (dword)&_int19Handler, ACS_INT);
    setIdtEntry(idt, 0x1A, 0x08, (dword)&_int1AHandler, ACS_INT);
    setIdtEntry(idt, 0x1B, 0x08, (dword)&_int1BHandler, ACS_INT);
    setIdtEntry(idt, 0x1C, 0x08, (dword)&_int1CHandler, ACS_INT);
    setIdtEntry(idt, 0x1D, 0x08, (dword)&_int1DHandler, ACS_INT);
    setIdtEntry(idt, 0x1E, 0x08, (dword)&_int1EHandler, ACS_INT);
    setIdtEntry(idt, 0x1F, 0x08, (dword)&_int1FHandler, ACS_INT);

    idtr.base = (dword) &idt;
    idtr.limit = sizeof(idt) - 1;

    _lidt(&idtr);

    setInterruptHandlerTable();

    /* Enable the interrupts we need in the PIC. */
    outB(0x21,0xFC);
    outB(0xA1,0xFF);

    // Enabling the interrupts again.
    _sti();
}


/**
 * Loads an interrupt in the IDT.
 *
 * @param table Adress of the IDT struct.
 * @param entry The number of the interrupt.
 * @param segmentSelector The selector of the segment in which the interrupt will be stored.
 * @param offset The offset to the exact location where the interrupt will be stored.
 * @param access The byte representing the type of access the segment has.
 */
void setIdtEntry(InterruptDescriptor* table, int entry, byte segmentSelector, dword offset, byte access) {
    InterruptDescriptor* item = &table[entry];

    item->selector = segmentSelector;
    item->offset_l = offset & 0xFFFF;
    item->offset_h = (word) (offset >> 16);
    item->access = access;
    item->cero = 0x00;
}

/**
 * Remaps the PIC.
 *
 * Initially, in real mode, the first 32 positions of the IDT are occupied by the exceptions
 * and the PIC's interrupts, IRQs. This function moves the IRQs in the IDT, so they
 *  don't overlap with the exceptions.
 *
 * @param offset1 The offset of the position where the IRQ0 was.
 * @param offset2 The offset of the position where the IRQ0 will be moved.
 */
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

