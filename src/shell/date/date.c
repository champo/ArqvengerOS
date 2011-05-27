#include "shell/date/date.h"
#include "library/stdio.h"
#include "library/string.h"
#include "mcurses/mcurses.h"
#include "system/rtc.h"
#include "library/time.h"

void date(char* argv) {
    
    RTCRegisters regs;
    readRTCRegisters(&regs);
    
    struct tm aux;
    aux.sec = regs.seconds;
    aux.min = regs.minutes; 
    aux.hour= regs.hours;
    aux.mday= regs.day;
    aux.mon = regs.month;
    aux.year= regs.year + regs.century*100;
    char *str = asctime(&aux);
    printf("%s",str);
}

void manDate(void) {
    setBold(1);
    printf("Usage:\n\tdate\n");
    setBold(0);
}
