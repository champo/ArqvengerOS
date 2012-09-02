#include "shell/getCPUSpeed/getCPUSpeed.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"

#define MEASURES_NUMBER		6

int getCPUSpeedHandler(void);

/**
 * Command that returns the CPU speed.
 *
 * It calculates a CPU speed making a measurement of clocks cycles and
 * diving by the time they took.
 *
 * @param argv A string containg everything that came after the command.
 */
void getCPUSpeed(char* argv) {

    UNUSED(argv);

    int i, acumMeasure = 0;
    for (i = 0; i < MEASURES_NUMBER;i++) {
        acumMeasure += getCPUSpeedHandler();
    }
    printf("The measured CPU Speed is: %d Mhz\n",acumMeasure / MEASURES_NUMBER);
}

/**
 * Print manual page for the getCPUSpee command.
 *
 */
void manGetCPUSpeed(void) {
    setBold(1);
    printf("Usage:\n\t getCPUSpeed\n");
    setBold(0);
}
