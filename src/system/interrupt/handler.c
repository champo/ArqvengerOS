#include "drivers/keyboard.h"
#include "system/tick.h"

void int08();

void int09();

void int08() {
    timerTick();
}

void int09() {
    readScanCode();
}
