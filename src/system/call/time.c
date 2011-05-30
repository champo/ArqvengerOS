#include "system/call.h"
#include "drivers/rtc.h"


/**
 * System call that returns the time since Epoch measured in seconds.
 *
 * Enables interaction between code running in user space and the RTC.
 * If the parameter tp is non-NULL, the return value is also stored in the
 * memory pointed to by tp.
 *
 * Epoch is considered to be 00:00:00 UTC, January 1, 1970.
 *
 * @param tp Pointer to a unsigned int where, if it is not NULL the result
 * 	     will be stored.
 *
 * @return The amount of second since Epoch.
 */
time_t _time(time_t *tp) {
    return getTime(tp);
}
