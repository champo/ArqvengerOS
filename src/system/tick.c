#include "system/tick.h"
#include "system/call.h"
#include "type.h"

static size_t ticksSinceStart = 0;

void timerTick() {
    ticksSinceStart++;
}

