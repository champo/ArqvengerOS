#include "shell/getCPUSpeed/getCPUSpeed.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"

int getCPUSpeedHandler(void);

void getCPUSpeed(char* argv) {
    int cpuSpeed = getCPUSpeedHandler();
    printf("The measured CPU Speed is: %d Mhz\n",cpuSpeed);
}

void manGetCPUSpeed(void) {
    setBold(1);
    printf("Usage:\n\t getCPUSpeed\n");
    setBold(0);
}
