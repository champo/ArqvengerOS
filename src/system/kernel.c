#include "system/interrupt.h"
#include "shell/shell.h"
#include "library/stdio.h"
#include "drivers/video.h"
#include "drivers/keyboard.h"
#include "system/mm.h"
#include "system/common.h"
#include "system/gdt.h"
#include "system/process/scheduler.h"

void kmain(struct multiboot_info* info, unsigned int magic);

static void idle(void) {
    while (1);
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

    initScreen();
    writeScreen("\033[1;1H\033[2J", 10);

    initKeyboard();
    initMemoryMap(info);

    disableInterrupts();
    struct Process* p = kalloc(sizeof(struct Process));
    createProcess(p, shell);
    scheduler_add(p);
    struct Process* p1 = kalloc(sizeof(struct Process));
    createProcess(p1, idle);
    scheduler_add(p1);
    enableInterrupts();

    while (1) {}
}


