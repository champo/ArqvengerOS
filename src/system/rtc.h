#ifndef __system_rtc_header
#define __system_rtc_header 

#include "type.h"

typedef struct {
    int seconds,minutes,hours,day,month,year,century;
} RTCRegisters;

void readRTCRegisters(RTCRegisters *regs);

time_t getTime(time_t *tp);

#endif
