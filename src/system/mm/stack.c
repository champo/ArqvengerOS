#include "system/mm/stack.h"
#include "system/kprintf.h"
#include "system/process/process.h"
#include "system/process/table.h"
#include "system/mm/pagination.h"
#include "system/mm/allocator.h"
#include "system/mm/page.h"

void page_fault_handler(int errCode) {
   
    struct Process* process = scheduler_current();

    if (errCode & 1 == 1) {
        kprintf("A process tried to access a page and caused a protection fault.\n");
        process_table_kill(process);
        return;
    }
    
    unsigned int to = 0;
    
    unsigned int last = STACK_TOP_MAPPING - process->mm.pagesInStack * PAGE_SIZE;

    __asm__ __volatile__ ("mov %%cr2, %%eax":"=A"(to)::);

    int difference = last - to;

    if (difference > PAGE_SIZE) {
        kprintf("A process tried to read a non-present page entry.\n");
        process_table_kill(process);
        return;
    }

    if (process->mm.pagesInStack >= MAX_PAGES_IN_STACK) {
        kprintf("Stack is way too big.\n");
        process_table_kill(process);
        return;
    }

    struct Pages* newPages = reserve_pages(process,1);

    if (newPages == NULL) {
        kprintf("Error. Couldn't reserve more pages for a process.\n");
        process_table_kill(process);
        return;
    }
    
    unsigned int start = (unsigned int)newPages->start;
 
    mm_pagination_map(process, start, to, 1, 1, 1);

    process->mm.pagesInStack++;

    int pages = process->mm.pagesInStack;

    log_debug("Growing stack of %d to %d.\n", process->pid, pages * PAGE_SIZE);
    log_debug("Now this stack contains %d pages.\n", pages);

    return;
}
