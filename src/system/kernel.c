#include "system/interrupt.h"
#include "shell/shell.h"
#include "library/stdio.h"
#include "drivers/video.h"
#include "drivers/keyboard.h"
#include "system/mm.h"

void kmain(struct multiboot_info* info, unsigned int magic);

/**
 * Kernel entry point
 */
void kmain(struct multiboot_info* info, unsigned int magic) {

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
    while (1) {
       shell();
    }
}

