#include "system/gdt.h"
#include "system/mm.h"
#include "system/common.h"

#pragma pack(1)

struct SegmentDescriptor {
    short limit;
    short base_l;
    char base_m;
    char access;
    char attribs;
    char base_h;
};

struct SegmentDescriptor* gdt;

struct GDTR {
    short limit;
    int base;
};

static void setupGDTEntry(int num, int base, int limit, short access, short gran) {
   gdt[num].base_l      = (base & 0xFFFF);
   gdt[num].base_m      = (base >> 16) & 0xFF;
   gdt[num].base_h      = (base >> 24) & 0xFF;

   gdt[num].limit       = (limit & 0xFFFF);
   gdt[num].attribs     = ((limit >> 16) & 0x0F) | (gran & 0xF0);
   gdt[num].access      = access;
}

void setupGDT(void) {

    struct GDTR gdtr;

    gdt = kalloc(sizeof(struct SegmentDescriptor) * 5);
    setupGDTEntry(0, 0, 0, 0, 0);
    setupGDTEntry(1, 0, 0x000FFFFF, 0x9A, 0xC0);
    setupGDTEntry(2, 0, 0x000FFFFF, 0x92, 0xC0);
    setupGDTEntry(3, 0, 0x000FFFFF, 0xFA, 0xC0);
    setupGDTEntry(4, 0, 0x000FFFFF, 0xF2, 0xC0);

    gdtr.limit = 5 * sizeof(struct SegmentDescriptor) - 1;
    gdtr.base = (int) gdt;

    __asm__ volatile("lgdt (%%eax)"::"A"(&gdtr):);
}

