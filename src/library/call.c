#include "library/call.h"

/**
 * This is the interface for the system call.
 *
 * @param eax, the code of the system call.
 * @param ebx, depends on the system call.
 * @param ecx, depends on the system call.
 * @param edx, depends on the system call.
 * @return depends on the system call.
 */

int system_call(int eax, int ebx, int ecx, int edx) {

    int ret;
    __asm__ __volatile__ ( "int $0x80"
            : "=a" (ret)
            : "a" (eax), "b" (ebx), "c" (ecx), "d" (edx)
    );

    return ret;
}
