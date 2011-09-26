#include "system/tick.h"
#include "system/call.h"
#include "type.h"

static size_t ticksSinceStart = 0;

/**
 * Increment the amoun of ticks since processor's start.
 *
 * Called when IRQ0 is triggered.
 *
 */
void timerTick(void) {
    ticksSinceStart++;
}

/**
 * A getter of the amount of ticks since processor's start.
 *
 * @return the amount of ticks since processor's start.
 */
size_t _getTicksSinceStart(void) {
    return ticksSinceStart;
}
