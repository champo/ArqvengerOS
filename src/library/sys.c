#include "library/sys.h"
#include "system/call/codes.h"
#include "library/call.h"

/**
 * Calls to a system call to yield the processor.
 */
void yield(void) {
    system_call(_SYS_YIELD, 0, 0, 0);
}

/**
 * Calls to a system call to wait for a child process to finish its execution.
 *
 * @return the pid of the child that has finished.
 */
pid_t wait(void) {
    return system_call(_SYS_WAIT, 0, 0, 0);
}

/**
 * Calls a system call to finish the process in a clean way.
 */
void exit(void) {
    system_call(_SYS_EXIT, 0, 0, 0);
}

/**
 * Calls a system call that runs a new process.
 *
 * @param entrypoint, the entrypoint of the new process.
 * @param args, the arguments that the new process receives.
 * @param fg, if the process runs on foreground.
 * @return the process id of the new process.
 */
pid_t run(void(*entryPoint)(char*), char* args, int fg) {
    return system_call(_SYS_RUN, (int) entryPoint, (int) args, fg);
}

/**
 * Calls a system call to get the process id.
 *
 * @return the process id.
 */
pid_t getpid(void) {
    return system_call(_SYS_GETPID, 0, 0, 0);
}

/**
 * Calls a system call to get the process id of the parent.
 *
 * @return the process id of the parent.
 */
pid_t getppid(void) {
    return system_call(_SYS_GETPPID, 0, 0, 0);
}

/**
 * Calls a system call to terminate a process.
 *
 * @param pid, the process id of the process to be terminated.
 */
void kill(pid_t pid) {
    system_call(_SYS_KILL, pid, 0, 0);
}

/**
 * Calls a system call that blocks the process a certain amount of time.
 *
 * @param seconds, the quantity of seconds to be blocked.
 */
void sleep(int seconds) {
    system_call(_SYS_SLEEP, seconds, 0, 0);
}

/**
 * Calls a system call that changes the priority of the the process that calls nice.
 *
 * @param priority, the new priority.
 * @return the new priority.
 */
int nice(int priority) {
    return system_call(_SYS_NICE, priority, 0, 0);
}

/** 
 * Calls a system call that changes the priority of a process.
 *
 * @param pid, the process id of the process to be changed.
 * @param priority, the new priority.
 * @return the new priority.
 */
int renice(int pid, int priority) {
    return system_call(_SYS_RENICE, pid, priority, 0);
}

/**
 *  Calls a system call that gets information of the processes.
 *
 *  @param data, an array of structures where the information will be returned.
 *  @param size, the lenght of the array.
 *  @return the quantity of structures read.
 */
int pinfo(struct ProcessInfo* data, size_t size) {
    return system_call(_SYS_PINFO, data, size, 0);
}

/**
 * Calls a system call that sets the gid and uid for a process.
 *
 * @param pid, the process id of the process to be changed.
 * @param gid, the new group id.
 * @param uid, the new user id.
 */
void setProcessPersona(int pid, int uid, int gid) {
    system_call(_SYS_SETPPERSONA, pid, uid, gid);
}

void getProcessPersona(int pid, int* uid, int* gid) {
    system_call(_SYS_GETPPERSONA, pid, uid, gid);
}
