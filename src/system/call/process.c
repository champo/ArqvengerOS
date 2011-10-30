#include "system/call.h"
#include "system/process/table.h"
#include "system/scheduler.h"
#include "type.h"

pid_t _getpid(void) {
    return scheduler_current()->pid;
}

pid_t _getppid(void) {
    return scheduler_current()->ppid;
}

pid_t _run(EntryPoint entryPoint, char* args, int active) {
    struct Process* parent = scheduler_current();
    struct Process* p = process_table_new(entryPoint, args, parent, 0, parent->terminal, active);
    return p->pid;
}

void _exit(void) {
    process_table_exit(scheduler_current());
}

pid_t _wait(void) {
    return process_table_wait(scheduler_current());
}

void _kill(pid_t pid) {
    struct Process* d = process_table_get(pid);
    if (d != NULL) {
        process_table_kill(d);
    }
}

void _sleep(int seconds) {
    scheduler_sleep(scheduler_current(), seconds);
}

int _nice(int priority) {
    struct Process* proc = scheduler_current();
    proc->schedule.priority = priority;
    return proc->schedule.priority;
}

int _renice(pid_t pid, int priority) {
    struct Process* proc = process_table_get(pid);
    
    if (proc == NULL) {             //this is priority dependent, must return 
        return INVALID_PRIORITY;    //a value which is invalid for a priority
    }

    proc->schedule.priority = priority;
    return proc->schedule.priority;
}
