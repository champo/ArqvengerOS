#include "drivers/keyboard.h"
#include "system/tick.h"
#include "system/call.h"
#include "system/io.h"

typedef struct {
   int gs, fs, es, ds;
   int edi, esi, ebp, esp, ebx, edx, ecx, eax;
   int intNum, errCode;
  // int eip, cs, eflags, useresp, ss;
} registers;

void int20(registers* regs);
void int21(registers* regs);
void int80(registers* regs);
void interruptDispatcher(registers regs);

void int20(registers* regs) {
    timerTick();
}

void int21(registers* regs ) {
    readScanCode();
}

typedef void (*interruptHandler)(registers* regs);

interruptHandler table[256];

void setInterruptHandlerTable() {
    table[128] = &int80;
    
    table[32] = &int20;
    table[33] = &int21;
}

#define PIC_MIN_INTNUM  32
#define PIC_IRQS        16
#define PIC_EOI         0x20
void interruptDispatcher(registers regs ){
    
    (*table[regs.intNum])(&regs);

    if ( regs.intNum >= PIC_MIN_INTNUM && regs.intNum < PIC_MIN_INTNUM + PIC_IRQS) {     
        if ( regs.intNum - PIC_MIN_INTNUM >= 8) {
            // Tell the slave PIC we're done
            outb(0xA0, PIC_EOI);
        }
        // Tell the master PIC we're done
        outB(0x20, PIC_EOI);
    }
}


#define     _SYS_READ   3
#define     _SYS_WRITE  4

void int80(registers* regs) {
    
    switch (regs->eax) {
        case _SYS_READ: 
            regs->eax = read((unsigned int)regs->ebx, (char*)regs->ecx, (size_t)regs->edx);
            break;
        case _SYS_WRITE:
            regs->eax = write((unsigned int)regs->ebx, (const char*)regs->ecx, (size_t)regs->edx);
            break;
    }
}
