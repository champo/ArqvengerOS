#include "system/panic.h"
#include "drivers/tty/tty.h"
#include "system/common.h"

void panic(void) {
    disableInterrupts();
    writeScreen("Kernel Panic", 11);
    halt();
}

