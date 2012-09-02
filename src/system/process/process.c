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
#include "library/stdio.h"
#include "system/gdt.h"
#include "system/interrupt.h"
#include "debug.h"
#include "system/mm/pagination.h"
#include "system/task.h"

static int pid = 0;

extern void _interruptEnd(void);

extern void signalPIC(void);

static void setup_page_directory(struct Process* process, int kernel);

inline static void push(unsigned int** esp, unsigned int val) {
    *esp -= 1;
    **esp = val;
}

void createProcess(struct Process* process, EntryPoint entryPoint, struct Process* parent, const char* args, int terminal, int kernel) {

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
        for (i = 0; *(args + i) && i < ARGV_SIZE - 1; i++) {
            process->args[i] = args[i];
        }
        process->args[i] = 0;
    }

    process->schedule.priority = 2;
    process->schedule.status = StatusReady;
    process->schedule.inWait = 0;
    process->schedule.ioWait = 0;
    process->schedule.asleep = 0;
    process->schedule.done = 0;

    for (size_t i = 0; i < MAX_OPEN_FILES; i++) {

        if (parent && parent->fdTable[i].inode) {
            fs_dup(&process->fdTable[i], parent->fdTable[i]);
        } else {
            process->fdTable[i].inode = NULL;
        }
    }

    process->mm.reservedPages = NULL;

    {
        process->mm.pagesInKernelStack = KERNEL_STACK_PAGES;
        struct Pages* mem = reserve_pages(process, process->mm.pagesInKernelStack);
        assert(mem != NULL);

        process->mm.esp0 = (char*)mem->start + PAGE_SIZE * process->mm.pagesInKernelStack;
        process->mm.kernelStack = process->mm.esp0;
    }

    if (kernel) {
        process->mm.pagesInHeap = 0;
        process->mm.mallocContext = NULL;
    } else {
        process->mm.pagesInHeap = 256;
        struct Pages* mem = reserve_pages(process, process->mm.pagesInHeap);
        assert(mem != NULL);

        process->mm.mallocContext = mm_create_context(mem->start, process->mm.pagesInHeap * PAGE_SIZE);
        mem_check();
    }

    if (!kernel) {
        process->mm.pagesInStack = 16;
        struct Pages* mem = reserve_pages(process, process->mm.pagesInStack);
        assert(mem != NULL);

        process->mm.esp = (char*)mem->start + PAGE_SIZE * process->mm.pagesInStack;
    } else {
        process->mm.pagesInStack = 0;
        process->mm.esp = NULL;
    }

    setup_page_directory(process, kernel);

    if (!kernel) {
        struct Pages* ungetPage = reserve_pages(process, 1);
        assert(ungetPage != NULL);
        mm_pagination_map(process, (unsigned int)ungetPage->start, (unsigned int)STACK_TOP_MAPPING, 1, 1, 1);

        for (int i = 0; i < 3; i++) {
            FILE* files = (FILE*) ungetPage->start + i;
            files->fd = i;
            files->flag = 0;
            files->unget = 0;
        }
    }

    int codeSegment, dataSegment;
    if (kernel) {
        codeSegment = KERNEL_CODE_SEGMENT;
        dataSegment = KERNEL_DATA_SEGMENT;

        char* esp0 = (char*) process->mm.esp0 - ARGV_SIZE;
        for (size_t i = 0; i < ARGV_SIZE; i++) {
            esp0[i] = process->args[i];
        }
        process->mm.esp0 = esp0;

        push((unsigned int**) &process->mm.esp0, (unsigned int) process->mm.esp0);
        push((unsigned int**) &process->mm.esp0, (unsigned int) exit);
        push((unsigned int**) &process->mm.esp0, 0x202);
    } else {
        codeSegment = USER_CODE_SEGMENT;
        dataSegment = USER_DATA_SEGMENT;

        char* esp = (char*) process->mm.esp - ARGV_SIZE;
        for (size_t i = 0; i < ARGV_SIZE; i++) {
            esp[i] = process->args[i];
        }
        process->mm.esp = esp;

        push((unsigned int**) &process->mm.esp, STACK_TOP_MAPPING - ARGV_SIZE);
        push((unsigned int**) &process->mm.esp, (unsigned int) exit);

        push((unsigned int**) &process->mm.esp0, dataSegment);
        push((unsigned int**) &process->mm.esp0, STACK_TOP_MAPPING - 2 * sizeof(int) - ARGV_SIZE);
        push((unsigned int**) &process->mm.esp0, 0x3202);
    }

    push((unsigned int**) &process->mm.esp0, codeSegment);
    push((unsigned int**) &process->mm.esp0, (unsigned int) entryPoint);
    push((unsigned int**) &process->mm.esp0, dataSegment);
    push((unsigned int**) &process->mm.esp0, dataSegment);
    push((unsigned int**) &process->mm.esp0, dataSegment);
    push((unsigned int**) &process->mm.esp0, dataSegment);
    push((unsigned int**) &process->mm.esp0, (unsigned int) _interruptEnd);
    push((unsigned int**) &process->mm.esp0, (unsigned int) signalPIC);
    push((unsigned int**) &process->mm.esp0, 0);
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
    if (process->mm.reservedPages) {
        free_pages(process->mm.reservedPages);
        process->mm.reservedPages = NULL;
    }
}

void setup_page_directory(struct Process* process, int kernel) {

    struct ProcessMemory* mm = &process->mm;
    if (kernel) {
        mm->directory = NULL;
        return;
    }

    struct Pages* pages = reserve_pages(process, 1);
    assert(pages != NULL);

    struct PageDirectory* directory = pages->start;
    mm_pagination_clear_directory(directory);
    process->mm.directory = directory;

    mm_pagination_map(process, 0, 0, 1024, 1, 1);

    unsigned int idt = idt_page_address();
    mm_pagination_map(process, idt, idt, 1, 0, 0);

    unsigned int gdt = gdt_page_address();
    mm_pagination_map(process, gdt, gdt, 1, 0, 0);

    unsigned int ts = task_page_address();
    mm_pagination_map(process, ts, ts, 1, 0, 0);

    unsigned int kernelStackBottom = (unsigned int) mm->kernelStack - mm->pagesInKernelStack * PAGE_SIZE;
    mm_pagination_map(process, kernelStackBottom, kernelStackBottom, mm->pagesInKernelStack, 0, 1);

    if (mm->mallocContext) {
        mm_pagination_map(process, (unsigned int) mm->mallocContext, (unsigned int) mm->mallocContext, mm->pagesInHeap, 1, 1);
    }

    if (mm->esp) {
        unsigned int stackBottom = (unsigned int) mm->esp - mm->pagesInStack * PAGE_SIZE;
        mm_pagination_map(process, stackBottom, STACK_TOP_MAPPING - mm->pagesInStack * PAGE_SIZE, mm->pagesInStack, 1, 1);
    }
}

