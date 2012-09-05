#include "system/gdt.h"
#include "system/mm/allocator.h"
#include "system/common.h"
#include "system/task.h"

#pragma pack(1)

// Do not ever remove this line.
// Removing it will cause death on optimization levels higher than 0
#pragma GCC optimize(0)

struct SegmentDescriptor {
    short limit;
    short base_l;
    char base_m;
    char access;
    char attribs;
    char base_h;
} __attribute__((packed));

static struct SegmentDescriptor* gdt;

struct GDTR {
    short limit;
    int base;
} __attribute__((packed));

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
    struct TaskState* ts = task_create_tss();

    gdt = allocPages(1);
    setupGDTEntry(0, 0, 0, 0, 0);
    setupGDTEntry(1, 0, 0x000FFFFF, 0x9A, 0xC0);
    setupGDTEntry(2, 0, 0x000FFFFF, 0x92, 0xC0);
    setupGDTEntry(3, 0, 0x000FFFFF, 0xFC, 0xC0);
    setupGDTEntry(4, 0, 0x000FFFFF, 0xF2, 0xC0);
    setupGDTEntry(5, (int) ts, (int) (ts + 1) + 1, 0x89, 0);

    gdtr.limit = 6 * sizeof(struct SegmentDescriptor) - 1;
    gdtr.base = (int) gdt;

    __asm__ volatile("lgdt (%%eax)"::"A"(&gdtr):);
    __asm__ volatile("ltr %%ax"::"a"(TASK_STATE_SEGMENT):);
}

unsigned int gdt_page_address(void) {
    return (unsigned int) gdt;
}

