#include "system/panic.h"
#include "drivers/tty/tty.h"
#include "system/common.h"

static const char* message = "Kernel Panic";
static char* screen = (char*) 0xb8000;

void panic(void) {
    disableInterrupts();
    for (int i = 0; i < 80 * 25; i++) {
        screen[2 * i] = ' ';
    }

    for (int i = 0; i < 12; i++) {
        screen[2 * i] = message[i];
    }

    halt();
}

