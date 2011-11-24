#include "system/interrupt.h"
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

void kmain(struct multiboot_info* info, unsigned int magic);

static void idle(char* unused) {
    while (1) {
        yield();
    }
}

/**
 * Kernel entry point
 */
void kmain(struct multiboot_info* info, unsigned int magic) {

    initMemoryMap(info);
    mm_pagination_init();

    setupGDT();
    setupIDT();

    FILE files[3];
    for (int i = 0; i < 3; i++) {
        files[i].fd = i;
        files[i].flag = 0;
        files[i].unget = 0;
    }

    stdin = &files[0];
    stdout = &files[1];
    stderr = &files[2];

    cache_init();
    ata_init(info);
    fs_load();
    fifo_init();
    scheduler_init();

    struct Process* idleProcess = process_table_new(idle, "idle", NULL, 1, NO_TERMINAL, 0);
    struct Process* shellProcess = process_table_new(tty_run, "tty", idleProcess, 1, NO_TERMINAL, 0);
    yield();

    while (1) {}
}


