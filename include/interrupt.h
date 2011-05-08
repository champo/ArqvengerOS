#ifndef _interrupt_header_
#define _interrupt_header_

#include "../include/type.h"

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

void setupIdtEntry(InterruptDescriptor* item, int entry, byte segmentSelector, dword offset, byte access);

void setupIDT();

#endif

