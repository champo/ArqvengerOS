#ifndef __LIBRARY_CALL__
#define __LIBRARY_CALL__

/**
 * This is the interface for the system call.
 *
 * @param eax, the code of the system call.
 * @param ebx, depends on the system call.
 * @param ecx, depends on the system call.
 * @param edx, depends on the system call.
 * @return depends on the system call.
 */
inline static int system_call(int eax, int ebx, int ecx, int edx) {

    int ret;
    __asm__ __volatile__ ( "int $0x80"
            : "=a" (ret)
            : "a" (eax), "b" (ebx), "c" (ecx), "d" (edx)
    );

    return ret;
}

#define SYS4(a, b, c, d) (system_call((int) (a), (int) (b), (int) (c), (int) (d)))
#define SYS3(a, b, c) SYS4(a, b, c, 0)
#define SYS2(a, b) SYS3(a, b, 0)
#define SYS1(a) SYS2(a, 0)

#endif
