#include "shell/getCPUSpeed/getCPUSpeed.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"

#include "system/rtc.h"
int getCPUSpeedHandler(void);

void getCPUSpeed(char* argv) {
    int cpuSpeed = getCPUSpeedHandler();
    printf("The measured CPU Speed is: %d Mhz\n",cpuSpeed);
    getTime(0);
}

void manGetCPUSpeed(void) {
    setBold(1);
    printf("Usage:\n\t getCPUSpeed\n");
    setBold(0);
}
