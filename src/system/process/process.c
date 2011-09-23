#include "system/process/process.h"
#include "system/mm.h"
#include "system/panic.h"
#include "system/common.h"
#include "system/process/scheduler.h"
#include "library/sys.h"

static int pid = 0;

extern void _interruptEnd(void);

extern void signalPIC(void);

inline static void push(int** esp, int val) {
    *esp -= 1;
    **esp = val;
}

void createProcess(struct Process* process, EntryPoint entryPoint, struct Process* parent, char* args) {

    process->pid = ++pid;
    process->parent = parent;
    process->entryPoint = entryPoint;
    process->children = 0;

    if (parent == NULL) {
        process->ppid = 0;
    } else {
        process->ppid = parent->pid;
        parent->children++;
    }

    if (args == NULL) {
        *process->args = 0;
    } else {
        int i;
        for (i = 0; *(args + i) && i < 255; i++) {
            process->args[i] = args[i];
        }
        process->args[i] = 0;
    }

    process->schedule.status = StatusReady;
    process->schedule.inWait = 0;
    process->schedule.done = 0;

    process->mm.pagesInStack = 256;
    process->mm.stackStart = allocPages(process->mm.pagesInStack);

    if (process->mm.stackStart == NULL) {
        panic();
    }

    process->mm.esp = (char*)process->mm.stackStart + PAGE_SIZE * process->mm.pagesInStack;
    push((int**) &process->mm.esp, (int) process->args);
    push((int**) &process->mm.esp, (int) exit);
    push((int**) &process->mm.esp, 0x200);
    push((int**) &process->mm.esp, 0x08);
    push((int**) &process->mm.esp, (int) entryPoint);
    push((int**) &process->mm.esp, (int) _interruptEnd);
    push((int**) &process->mm.esp, (int) signalPIC);
    push((int**) &process->mm.esp, 0);
}

void exitProcess(struct Process* process) {
    process->schedule.done = 1;
    if (process->mm.stackStart) {
        freePages(process->mm.stackStart, process->mm.pagesInStack);
        process->mm.stackStart = NULL;
    }
}

void destroyProcess(struct Process* process) {
    process->pid = 0;
    exitProcess(process);
}

