#include "system/process/process.h"
#include "system/mm/allocator.h"
#include "system/panic.h"
#include "system/common.h"
#include "system/scheduler.h"
#include "system/call.h"
#include "system/fs/fs.h"
#include "system/malloc/malloc.h"
#include "system/kprintf.h"
#include "library/sys.h"
#include "library/string.h"
#include "system/gdt.h"
#include "debug.h"

static int pid = 0;

extern void _interruptEnd(void);

extern void signalPIC(void);

inline static void push(int** esp, int val) {
    *esp -= 1;
    **esp = val;
}

void createProcess(struct Process* process, EntryPoint entryPoint, struct Process* parent, char* args, int terminal, int kernel) {

    process->pid = ++pid;
    process->kernel = !!kernel;
    process->terminal = terminal;
    process->active = 0;

    process->parent = parent;
    process->firstChild = NULL;

    process->cycles = 0;
    process->curr_cycles = 0;
    process->prev_cycles = 0;
    process->timeStart = _time(NULL);

    process->uid = 0;
    process->gid = 0;
    if (parent != NULL) {
        process->uid = parent->uid;
        process->gid = parent->gid;
    }

    process->prev = NULL;
    if (parent == NULL) {
        process->ppid = 0;
        process->next = NULL;

        process->cwd = kalloc(2 * sizeof(char));
        strcpy(process->cwd, "/");
    } else {
        process->ppid = parent->pid;

        process->next = parent->firstChild;
        if (parent->firstChild) {
            parent->firstChild->prev = process;
        }

        parent->firstChild = process;

        process->cwd = kalloc(strlen(parent->cwd) + 1);
        strcpy(process->cwd, parent->cwd);
    }

    process->entryPoint = entryPoint;
    if (args == NULL) {
        process->args[0] = 0;
    } else {
        int i;
        for (i = 0; *(args + i) && i < 255; i++) {
            process->args[i] = args[i];
        }
        process->args[i] = 0;
    }

    process->schedule.priority = 2;
    process->schedule.status = StatusReady;
    process->schedule.inWait = 0;
    process->schedule.ioWait = 0;
    process->schedule.done = 0;

    for (size_t i = 0; i < MAX_OPEN_FILES; i++) {

        if (parent && parent->fdTable[i].inode) {
            process->fdTable[i] = fs_dup(parent->fdTable[i]);
        } else {
            process->fdTable[i].inode = NULL;
        }
    }

    process->mm.pagesInKernelStack = KERNEL_STACK_PAGES;
    process->mm.kernelStackStart = allocPages(process->mm.pagesInKernelStack);
    process->mm.esp0 = (char*)process->mm.kernelStackStart + PAGE_SIZE * process->mm.pagesInKernelStack;
    process->mm.kernelStack = process->mm.esp0;

    if (kernel) {
        process->mm.pagesInHeap = 0;
        process->mm.heap = NULL;
        process->mm.mallocContext = NULL;
    } else {
        process->mm.pagesInHeap = 256;
        process->mm.heap = allocPages(process->mm.pagesInHeap);
        process->mm.mallocContext = mm_create_context(process->mm.heap, process->mm.pagesInHeap * PAGE_SIZE);

        mem_check();
    }

    if (!kernel) {
        process->mm.pagesInStack = 256;
        process->mm.stackStart = allocPages(process->mm.pagesInStack);
        if (process->mm.stackStart == NULL) {
            panic();
        }

        process->mm.esp = (char*)process->mm.stackStart + PAGE_SIZE * process->mm.pagesInStack;
        push((int**) &process->mm.esp, (int) process->args);
        push((int**) &process->mm.esp, (int) exit);
    } else {
        process->mm.pagesInStack = 0;
        process->mm.stackStart = NULL;
        process->mm.esp = NULL;
    }


    int codeSegment, dataSegment;
    if (kernel) {
        codeSegment = KERNEL_CODE_SEGMENT;
        dataSegment = KERNEL_DATA_SEGMENT;

        push((int**) &process->mm.esp0, (int) process->args);
        push((int**) &process->mm.esp0, (int) exit);
        push((int**) &process->mm.esp0, 0x202);
    } else {
        codeSegment = USER_CODE_SEGMENT;
        dataSegment = USER_DATA_SEGMENT;

        push((int**) &process->mm.esp0, dataSegment);
        push((int**) &process->mm.esp0, (int) process->mm.esp);
        push((int**) &process->mm.esp0, 0x3202);
    }

    push((int**) &process->mm.esp0, codeSegment);
    push((int**) &process->mm.esp0, (int) entryPoint);
    push((int**) &process->mm.esp0, dataSegment);
    push((int**) &process->mm.esp0, dataSegment);
    push((int**) &process->mm.esp0, dataSegment);
    push((int**) &process->mm.esp0, dataSegment);
    push((int**) &process->mm.esp0, (int) _interruptEnd);
    push((int**) &process->mm.esp0, (int) signalPIC);
    push((int**) &process->mm.esp0, 0);
}

void exitProcess(struct Process* process) {
    kfree(process->cwd);

    for (size_t i = 0; i < MAX_OPEN_FILES; i++) {

        if (process->fdTable[i].inode != NULL) {
            fs_fd_close(&process->fdTable[i]);
        }
    }
}

void destroyProcess(struct Process* process) {
    process->pid = 0;
    if (process->mm.stackStart) {
        freePages(process->mm.stackStart, process->mm.pagesInStack);
        process->mm.stackStart = NULL;
    }

    if (process->mm.kernelStackStart) {
        freePages(process->mm.kernelStackStart, process->mm.pagesInKernelStack);
        process->mm.kernelStackStart = NULL;
        process->mm.kernelStack = NULL;
        process->mm.esp0 = NULL;
    }

    if (process->mm.heap) {
        mm_set_process_context();
        mem_check();
        mm_set_kernel_context();
        mem_check();

        freePages(process->mm.heap, process->mm.pagesInHeap);
        process->mm.heap = NULL;
        process->mm.mallocContext = NULL;
    }
}

