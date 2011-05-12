#include "system/tick.h"
#include "common.h"

size_t ticksSinceStart = 0;

void timerTick() {
    ticksSinceStart++;
}
