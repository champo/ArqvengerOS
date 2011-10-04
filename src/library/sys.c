#include "library/sys.h"
#include "system/call/codes.h"
#include "library/call.h"

void yield(void) {
    system_call(_SYS_YIELD, 0, 0, 0);
}

pid_t wait(void) {
    return system_call(_SYS_WAIT, 0, 0, 0);
}

void exit(void) {
    system_call(_SYS_EXIT, 0, 0, 0);
}

pid_t run(void(*entryPoint)(char*), char* args, int fg) {
    return system_call(_SYS_RUN, (int) entryPoint, (int) args, fg);
}

pid_t getpid(void) {
    return system_call(_SYS_GETPID, 0, 0, 0);
}

pid_t getppid(void) {
    return system_call(_SYS_GETPPID, 0, 0, 0);
}

void kill(pid_t pid) {
    system_call(_SYS_KILL, pid, 0, 0);
}

