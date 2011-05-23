#include "system/call.h"
#include "system/rtc.h"

time_t _time(time_t *tp) {
    return getTime(tp);
} 
