#include "library/time.h"
#include "system/call/codes.h"

time_t time(time_t *tp) {
   systemCall(_SYS_TIME,tp,0,0);
}
