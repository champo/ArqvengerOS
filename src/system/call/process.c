#include "system/call.h"
#include "system/process/table.h"
#include "system/scheduler.h"
#include "type.h"

/**
 * Gets the process id.
 *
 * @return the process id.
 */
pid_t _getpid(void) {
    return scheduler_current()->pid;
}

/**
 * Gets the process id of the parent.
 *
 * @return the process id of the parent.
 */
pid_t _getppid(void) {
    return scheduler_current()->ppid;
}

/**
 * Runs a new process.
 *
 * @param entrypoint, the entrypoint of the new process.
 * @param args, the arguments that the new process receives.
 * @param active, if the process runs on foreground.
 * @return the process id of the new process.
 */
pid_t _run(EntryPoint entryPoint, char* args, int active) {
    struct Process* parent = scheduler_current();
    struct Process* p = process_table_new(entryPoint, args, parent, 0, parent->terminal, active);
    return p->pid;
}

/**
 * Finishes the process in a clean way.
 */
void _exit(void) {
    process_table_exit(scheduler_current());
}

/**
 * Waits for a child process to finish its execution.
 *
 * @return the pid of the child that has finished.
 */
pid_t _wait(void) {
    return process_table_wait(scheduler_current());
}

/**
 * Terminates a process.
 *
 * @param pid, the process id of the process to be terminated.
 */
void _kill(pid_t pid) {
    struct Process* d = process_table_get(pid);
    if (d != NULL) {
        process_table_kill(d);
    }
}

/**
 * Blocks the process a certain amount of time.
 *
 * @param seconds, the quantity of seconds to be blocked.
 */
void _sleep(int seconds) {
    scheduler_sleep(scheduler_current(), seconds);
}

/**
 * Changes the priority of the the process that calls nice.
 *
 * @param priority, the new priority.
 * @return the new priority.
 */
int _nice(int priority) {
    struct Process* proc = scheduler_current();
    proc->schedule.priority = priority;
    return proc->schedule.priority;
}

/** 
 * Changes the priority of a process.
 *
 * @param pid, the process id of the process to be changed.
 * @param priority, the new priority.
 * @return the new priority.
 */
int _renice(pid_t pid, int priority) {
    struct Process* proc = process_table_get(pid);
    
    if (proc == NULL) {             //this is priority dependent, must return 
        return INVALID_PRIORITY;    //a value which is invalid for a priority
    }

    proc->schedule.priority = priority;
    return proc->schedule.priority;
}
