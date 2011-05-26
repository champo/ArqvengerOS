#include "library/time.h"
#include "system/call/codes.h"

extern size_t systemCall(int eax, int ebx, int ecx, int edx);

time_t time(time_t *tp) {
    return systemCall(_SYS_TIME,tp,0,0);
}
