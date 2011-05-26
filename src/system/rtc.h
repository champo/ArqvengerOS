#ifndef __system_rtc_header
#define __system_rtc_header 

#include "type.h"

time_t getTime(time_t *tp);
int readSeconds(void);
int readMinutes(void);
int readHours(void);
int readDay(void);
int readMonth(void);
int readYear(void);
int readCentury(void);

#endif
