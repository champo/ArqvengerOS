#include "system/call.h"
#include "system/rtc.h"


/**
 * System call that returns the time since Epoch (00:00:00 UTC, January 1, 1970),
 *  measured in seconds.
 *
 * Enables interaction between code running in user space and the RTC.
 * If the parameter tp is non-NULL, the return value is also stored in the 
 * memory pointed to by tp.
 *
 * @param tp Pointer to a unsigned int where the result
 */
time_t _time(time_t *tp) {
    return getTime(tp);
} 
