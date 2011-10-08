#include "system/tick.h"
#include "system/call.h"
#include "system/process/table.h"
#include "system/scheduler.h"
#include "type.h"

static size_t ticksSinceStart = 0;

static long int tickCounter = 0;

/**
 * Increment the amoun of ticks since processor's start.
 *
 * Called when IRQ0 is triggered.
 *
 */
void timerTick(void) {
    ticksSinceStart++;

    tickCounter++;
    if (tickCounter > TICKS_SAMPLE_SIZE) {
        process_table_reset_cycles(); 
        tickCounter = 0;
        scheduler_tick();
    }
        
}

/**
 * A getter of the amount of ticks since processor's start.
 *
 * @return the amount of ticks since processor's start.
 */
size_t _getTicksSinceStart(void) {
    return ticksSinceStart;
}
