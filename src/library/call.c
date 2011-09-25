#include "library/call.h"

int system_call(int eax, int ebx, int ecx, int edx) {

    int ret;
    __asm__ __volatile__ ( "int $0x80"
            : "=a" (ret)
            : "0" (eax), "b" (ebx), "c" (ecx), "d" (edx)
    );

    return ret;
}
