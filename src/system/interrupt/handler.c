#include "drivers/keyboard.h"
#include "system/tick.h"
#include "system/call.h"
#include "system/io.h"
void int08();

void int09();
void int80();
void interruptDispatcher();

void int08() {
    char* str;
    timerTick();
    read(1,str,1);
}

void int09() {
    readScanCode();
}


typedef struct {
   int gs, fs, es, ds;
   int edi, esi, ebp, esp, ebx, edx, ecx, eax;
   int intNum, errCode;
  // int eip, cs, eflags, useresp, ss;
} registers;

typedef void (*interruptHandler)(registers* regs);

interruptHandler table[256];

void setInterruptHandlerTable(){
    table[80] = &int80;
    
    table[32] = &int08;
    table[33] = &int09;
}

#define PIC_MIN_INTNUM  32
#define PIC_IRQS        16

void interruptDispatcher(registers regs ){
    
    if(regs.intNum >= PIC_MIN_INTNUM && regs.intNum < PIC_MIN_INTNUM + PIC_IRQS ) {
        outB(0x20,0x20);
    }
    (*table[regs.intNum])(&regs);
}


#define     _SYS_READ   3
#define     _SYS_WRITE  4

void int80(registers* regs) {
    
    switch(regs->eax) {
        case _SYS_READ: 
            regs->eax = read((unsigned int)regs->ebx, (char*)regs->ecx, (size_t)regs->edx);
            break;
        case _SYS_WRITE:
            regs->eax = write((unsigned int)regs->ebx, (const char*)regs->ecx, (size_t)regs->edx);
            break;
    }
}
