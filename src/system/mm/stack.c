#include "system/mm/stack.h"
#include "system/kprintf.h"
#include "system/process/process.h"
#include "system/process/table.h"
#include "system/mm/pagination.h"
#include "system/mm/allocator.h"
#include "system/mm/page.h"
#include "system/scheduler.h"

#define PAGES_PER_INCREMENT 4

void page_fault_handler(int errCode) {

    struct Process* process = scheduler_current();

    if ((errCode & 1) == 1) {
        tkprintf(process->terminal, "A process tried to access a page and caused a protection fault.\n");
        process_table_kill(process);
        return;
    }

    unsigned int to = 0;

    unsigned int last = STACK_TOP_MAPPING - process->mm.pagesInStack * PAGE_SIZE;

    __asm__ __volatile__ ("mov %%cr2, %%eax":"=A"(to)::);

    if (last < to || last > to + PAGES_PER_INCREMENT * PAGE_SIZE) {
        tkprintf(process->terminal, "A process tried to read a non-present page entry.\n");
        process_table_kill(process);
        return;
    }

    if (process->mm.pagesInStack + PAGES_PER_INCREMENT > MAX_PAGES_IN_STACK) {
        tkprintf(process->terminal, "Stack is way too big.\n");
        process_table_kill(process);
        return;
    }

    struct Pages* newPages = reserve_pages(process, PAGES_PER_INCREMENT);

    if (newPages == NULL) {
        tkprintf(process->terminal, "Error. Couldn't reserve more pages for a process.\n");
        process_table_kill(process);
        return;
    }

    unsigned int start = (unsigned int)newPages->start;
    mm_pagination_map(process, start, last - PAGES_PER_INCREMENT * PAGE_SIZE, PAGES_PER_INCREMENT, 1, 1);

    process->mm.pagesInStack += PAGES_PER_INCREMENT;

    int pages = process->mm.pagesInStack;

    log_debug("Growing stack of process of pid %d to %d bytes (%d pages).\n", process->pid, pages * PAGE_SIZE, pages);
}

