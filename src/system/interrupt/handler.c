#include "drivers/keyboard.h"
#include "drivers/tty/tty.h"
#include "system/tick.h"
#include "system/call.h"
#include "system/io.h"
#include "system/common.h"
#include "system/interrupt/handler.h"
#include "system/call/codes.h"

typedef struct {
    int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    int intNum, errCode;
    int eip, cs, eflags, useresp, ss;
} registers;

typedef void (*interruptHandler)(registers* regs);

static int intNum;

static interruptHandler table[256];

#define     register(X)         table[0x##X] = &int##X

#define     PIC_MIN_INTNUM      32
#define     PIC_IRQS            16
#define     PIC_EOI             0x20

#define     IN_USE_EXCEPTIONS   20

static const char* exceptionTable[] = { "Divide by zero", "Debugger", "NMI", "Breakpoint", "Overflow",
                                         "Bounds", "Invalid Opcode", "Coprocesor not available",
                                         "Double fault", "Coprocessor Segment Overrun",
                                         "Invalid Task State Segment", "Segment not present",
                                         "Stack fault", "General Protection", "Page Fault",
                                         "Intel Reserved", "Math Fault", "Aligment Check",
                                         "Machine Check", "Floating-Point Exception"};

static void int20(registers* regs);
static void int21(registers* regs);
static void int80(registers* regs);
static void exceptionHandler(registers* regs);
void interruptDispatcher(registers regs);



/**
 * Interrupt 20h. Handles the IRQ0 also know as timmer tick.
 *
 *  @param regs Pointer to struct containing micro's registers.
 */
void int20(registers* regs) {
    timerTick();
}

/**
 * Interrupt 21h. Handles the IRQ1, the interrupt from keyboard.
 *
 *  @param regs Pointer to struct containing micro's registers.
 */
void int21(registers* regs ) {
    keyboard_read();
    process_scancode();
}

/**
 * Register interrupts in the handler table.
 *
 */
void setInterruptHandlerTable(void) {
    int i;
    for (i = 0;i < 32;i++) {
        table[i] = &exceptionHandler;
    }
    register(20);
    register(21);

    register(80);
}

/**
 * Call the apropiate interrupt.
 *
 * It makes use of the HandlerTable, in which previously all interrupts were
 * registered. In the case of a PIC interrupt, it informs the corresponding
 * PIC when the interrupt has been handled.
 *
 *  @param regs Pointer to struct containing micro's registers.
 */
void interruptDispatcher(registers regs) {

    // So, since the stack might be changed by a context change
    // We need access to this number some other way
    // And since the kernel itself is not preemptive, storing it like this is safe.
    intNum = regs.intNum;
    (*table[regs.intNum])(&regs);

    scheduler_do();
    signalPIC();
}

void signalPIC(void) {

    if (intNum >= PIC_MIN_INTNUM && intNum < PIC_MIN_INTNUM + PIC_IRQS) {
        if (intNum - PIC_MIN_INTNUM >= 8) {
            // Tell the slave PIC we're done
            outB(0xA0, PIC_EOI);
        }

        // Tell the master PIC we're done
        outB(0x20, PIC_EOI);
    }
}

/**
 * Interrupt 80h. Handles the system calls.
 *
 *  @param regs Pointer to struct containing micro's registers.
 */
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
        case _SYS_YIELD:
            // This just makes sure we call the scheduler again, for now
            break;
        case _SYS_EXIT:
            _exit();
            break;
        case _SYS_GETPID:
            regs->eax = _getpid();
            break;
        case _SYS_GETPPID:
            regs->eax = _getppid();
            break;
        case _SYS_RUN:
            regs->eax = _run((void(*)(char*)) regs->ebx, (char*) regs->ecx, regs->edx);
            break;
        case _SYS_WAIT:
            regs->eax = _wait();
            break;
        case _SYS_KILL:
            _kill((pid_t) regs->ebx);
            break;
        case _SYS_PINFO:
            regs->eax = _pinfo(regs->ebx, (size_t)regs->ecx);
            break;
    }
}

/**
 * Handles the exceptions.
 *
 *  @param regs Pointer to struct containing micro's registers.
 */
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

