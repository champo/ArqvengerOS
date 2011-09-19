#include "system/panic.h"
#include "drivers/video.h"
#include "system/common.h"

void panic(void) {
    disableInterrupts();
    writeScreen("\033[1;1H\033[2J", 10);
    writeScreen("Kernel Panic", 11);
    halt();
}

