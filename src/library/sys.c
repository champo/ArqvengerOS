#include "library/sys.h"
#include "system/call/codes.h"

extern size_t systemCall(int eax, int ebx, int ecx, int edx);

void yield(void) {
    systemCall(_SYS_YIELD, 0, 0, 0);
}

void wait(void) {
    systemCall(_SYS_WAIT, 0, 0, 0);
}

void exit(void) {
    systemCall(_SYS_EXIT, 0, 0, 0);
}

pid_t run(void(*entryPoint)(char*), char* args) {
    return systemCall(_SYS_RUN, (int) entryPoint, (int) args, 0);
}

pid_t getpid(void) {
    return systemCall(_SYS_GETPID, 0, 0, 0);
}

pid_t getppid(void) {
    return systemCall(_SYS_GETPPID, 0, 0, 0);
}

