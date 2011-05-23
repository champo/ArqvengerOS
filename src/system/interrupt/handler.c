#include "drivers/keyboard.h"
#include "system/tick.h"
#include "system/call.h"
#include "system/io.h"
#include "system/common.h"
#include "system/interrupt/handler.h"i
#include "system/call/codes.h"

typedef struct {
   int gs, fs, es, ds;
   int edi, esi, ebp, esp, ebx, edx, ecx, eax;
   int intNum, errCode;
  // int eip, cs, eflags, useresp, ss;
} registers;

typedef void (*interruptHandler)(registers* regs);

static interruptHandler table[256];

#define     register(X)         table[0x##X] = &int##X

#define     PIC_MIN_INTNUM      32
#define     PIC_IRQS            16
#define     PIC_EOI             0x20

#define     IN_USE_EXCEPTIONS   20

static const char* exceptionTable[] = { "Divide by zero", "Debugger", "NMI", "Breakpoint", "Overflow", "Bounds", "Invalid Opcode", "Coprocesor not available", "Double fault", "Coprocessor Segment Overrun", "Invalid Task State Segment", "Segment not present", "Stack fault", "General Protection", "Page Fault", "Intel Reserved", "Math Fault", "Aligment Check", "Machine Check", "Floating-Point Exception"};

static void int20(registers* regs);
static void int21(registers* regs);
static void int80(registers* regs);
static void exceptionHandler(registers* regs);
void interruptDispatcher(registers regs);

void int20(registers* regs) {
    timerTick();
}

void int21(registers* regs ) {
    readScanCode();
}

void setInterruptHandlerTable(void) {
    int i;
    for (i = 0;i < 32;i++) {
        table[i] = &exceptionHandler;
    }
    register(20);
    register(21);

    register(80);
}

void interruptDispatcher(registers regs) {

    (*table[regs.intNum])(&regs);

    if (regs.intNum >= PIC_MIN_INTNUM && regs.intNum < PIC_MIN_INTNUM + PIC_IRQS) {
        if (regs.intNum - PIC_MIN_INTNUM >= 8) {
            // Tell the slave PIC we're done
            outB(0xA0, PIC_EOI);
        }

        // Tell the master PIC we're done
        outB(0x20, PIC_EOI);
    }
}

void int80(registers* regs) {

    switch (regs->eax) {

        case _SYS_READ:
            regs->eax = _read((unsigned int)regs->ebx, (char*)regs->ecx, (size_t)regs->edx);
            break;
        case _SYS_WRITE:
            regs->eax = _write((unsigned int)regs->ebx, (const char*)regs->ecx, (size_t)regs->edx);
            break;
        case _SYS_TIME:
            regs->eax = _time(regs->ebx);
            break;
        case _SYS_IOCTL:
            regs->eax = _ioctl(regs->ebx, regs->ecx, (void*)regs->edx);
            break;
        case _SYS_TICKS:
            regs->eax = _getTicksSinceStart();
            break;
    }
}

void exceptionHandler(registers* regs){
    char* screen = (char*) 0xb8000;
    int i = 0;
    const char* message = "Unhandled CPU exception: ";
    while (i != 2*80*25) {
        *screen = ' ';
        screen += 2;
        i += 2;
    }
    i = 0;
    screen = (char*) 0xb8000;
    while ( message[i] != '\0') {
        *screen = message[i++];
        screen++;
        *screen = 0x07;
        screen++;
    }

    *screen = (regs->intNum/10)+'0';
    screen += 2;
    *screen = (regs->intNum%10)+'0';
    screen += 2;
    *screen = ' ';

    const char* exception;
    if (regs->intNum > IN_USE_EXCEPTIONS ){
        exception = "Reserved for future use";
    } else {
        exception = exceptionTable[regs->intNum];
    }
    i = 0;
    while ( exception[i] != '\0') {
        screen += 2;
        *screen = exception[i++];
    }

    disableInterrupts();
    halt();
}

