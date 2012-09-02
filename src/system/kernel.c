#include "system/interrupt.h"
#include "system/fs/fs.h"
#include "library/stdio.h"
#include "drivers/tty/tty.h"
#include "system/mm/allocator.h"
#include "system/gdt.h"
#include "system/process/table.h"
#include "system/scheduler.h"
#include "drivers/ata.h"
#include "system/fifo.h"
#include "system/mm/pagination.h"
#include "system/cache/cache.h"
#include "system/process/process.h"
#include "library/sys.h"

void kmain(struct multiboot_info* info, unsigned int magic);

static void idle(char* unused) {

    UNUSED(unused);

    while (1) {
        yield();
    }
}

/**
 * Kernel entry point
 */
void kmain(struct multiboot_info* info, unsigned int magic) {

    UNUSED(magic);

    initMemoryMap(info);
    mm_pagination_init();

    setupGDT();
    setupIDT();

    stdin = (FILE*) STACK_TOP_MAPPING;
    stdout = (FILE*) (STACK_TOP_MAPPING + sizeof(FILE));
    stderr = (FILE*) (STACK_TOP_MAPPING + 2 * sizeof(FILE));

    tty_early_init();

    cache_init();
    ata_init(info);
    fs_load();
    fifo_init();
    scheduler_init();

    struct Process* idleProcess = process_table_new(idle, "idle", NULL, 1, NO_TERMINAL, 0);
    process_table_new(tty_run, "tty", idleProcess, 1, NO_TERMINAL, 0);
    process_table_new(cache_flush, "cache_flush", idleProcess, 1, NO_TERMINAL, 0);
    yield();

    while (1) {}
}


