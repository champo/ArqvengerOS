#include "system/tick.h"
#include "type.h"

size_t ticksSinceStart = 0;

void timerTick() {
    ticksSinceStart++;
}
