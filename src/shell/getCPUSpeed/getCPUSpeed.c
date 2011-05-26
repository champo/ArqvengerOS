#include "shell/getCPUSpeed/getCPUSpeed.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"

#include "system/rtc.h"
#include "library/time.h"
int getCPUSpeedHandler(void);

void getCPUSpeed(char* argv) {
    int cpuSpeed = getCPUSpeedHandler();
    printf("The measured CPU Speed is: %d Mhz\n",cpuSpeed);
    struct tm aux;
    aux.sec = readSeconds();
    aux.min = readMinutes();
    aux.hour= readHours();
    aux.mday= readDay();
    aux.mon = readMonth();
    aux.year= readYear() + readCentury()*100;
   
    //TODO OJO CON EL Wday
    aux.wday = 4;
    char *str = asctime(&aux);
}

void manGetCPUSpeed(void) {
    setBold(1);
    printf("Usage:\n\t getCPUSpeed\n");
    setBold(0);
}
