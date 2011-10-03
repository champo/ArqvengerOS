#include "system/process/process.h"
#include "system/mm.h"
#include "system/panic.h"
#include "system/common.h"
#include "system/scheduler.h"
#include "library/sys.h"

static int pid = 0;

extern void _interruptEnd(void);

extern void signalPIC(void);

inline static void push(int** esp, int val) {
    *esp -= 1;
    **esp = val;
}

void createProcess(struct Process* process, EntryPoint entryPoint, struct Process* parent, char* args, int terminal, int active) {

    process->pid = ++pid;
    process->terminal = terminal;
    process->active = active;

    process->parent = parent;
    process->firstChild = NULL;

    process->prev = NULL;
    if (parent == NULL) {
        process->ppid = 0;

        process->next = NULL;
    } else {
        process->ppid = parent->pid;

        process->next = parent->firstChild;
        if (parent->firstChild) {
            parent->firstChild->prev = process;
        }

        parent->firstChild = process;
    }

    process->entryPoint = entryPoint;
    if (args == NULL) {
        *process->args = 0;
    } else {
        int i;
        for (i = 0; *(args + i) && i < 255; i++) {
            process->args[i] = args[i];
        }
        process->args[i] = 0;
    }

    process->schedule.priority = 0;
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
}

void destroyProcess(struct Process* process) {
    process->pid = 0;
    if (process->mm.stackStart) {
        freePages(process->mm.stackStart, process->mm.pagesInStack);
        process->mm.stackStart = NULL;
    }
    exitProcess(process);
}

