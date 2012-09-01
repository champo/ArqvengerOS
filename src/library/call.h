#ifndef __LIBRARY_CALL__
#define __LIBRARY_CALL__

inline int system_call(int eax, int ebx, int ecx, int edx);

#define SYS4(a, b, c, d) (system_call((int) (a), (int) (b), (int) (c), (int) (d)))
#define SYS3(a, b, c) SYS4(a, b, c, 0)
#define SYS2(a, b) SYS3(a, b, 0)
#define SYS1(a) SYS2(a, 0)

#endif
