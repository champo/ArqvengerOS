#include "system/process/process.h"
#include "system/mm.h"
#include "system/panic.h"
#include "system/common.h"

static int pid = 0;

extern void _interruptEnd(void);
extern void _l(void);

extern void signalPIC(void);

inline static void push(int** esp, int val) {
    *esp -= 1;
    **esp = val;
}

static void _exit(void) {
    //TODO: Kill the bastard
    writeScreen("E", 1);
    while (1);
}

void createProcess(struct Process* process, void* entrypoint) {
    process->pid = ++pid;

    process->mm.pagesInStack = 256;
    process->mm.stackStart = allocPages(process->mm.pagesInStack);

    if (process->mm.stackStart == NULL) {
        panic();
    }

    process->mm.esp = (char*)process->mm.stackStart + PAGE_SIZE * process->mm.pagesInStack;
    push((int**) &process->mm.esp, (int) _exit);
    push((int**) &process->mm.esp, 0x200);
    push((int**) &process->mm.esp, 0x08);
    push((int**) &process->mm.esp, (int) entrypoint);
    push((int**) &process->mm.esp, (int) _interruptEnd);
    push((int**) &process->mm.esp, (int) signalPIC);
    push((int**) &process->mm.esp, 0);
}

void destroyProcess(struct Process* process) {
    process->pid = 0;
    freePages(process->mm.stackStart, process->mm.pagesInStack);
}

