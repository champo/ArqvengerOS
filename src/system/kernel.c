#include "system/interrupt.h"
#include "shell/shell.h"
#include "library/stdio.h"
#include "library/string.h"
#include "drivers/tty/tty.h"
#include "system/mm.h"
#include "system/common.h"
#include "system/gdt.h"
#include "system/process/table.h"
#include "system/scheduler.h"
#include "system/accessControlList/users.h"
#include "system/accessControlList/groups.h"
#include "drivers/ata.h"

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

    initMemoryMap(info);
    ata_init(info);
    scheduler_init();
    create_user("root", "root");
    create_group("root");
    add_group_member(0,0);
    create_group("users");

    struct Process* idleProcess = process_table_new(idle, NULL, NULL, 1, NO_TERMINAL, 0);
    struct Process* shellProcess = process_table_new(tty_run, NULL, idleProcess, 1, NO_TERMINAL, 0);
    enableInterrupts();

    while (1) {}
}


