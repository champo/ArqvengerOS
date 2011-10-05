#include "system/call.h"
#include "system/process/table.h"
#include "system/scheduler.h"

pid_t _getpid(void) {
    return scheduler_current()->pid;
}

pid_t _getppid(void) {
    return scheduler_current()->ppid;
}

pid_t _run(EntryPoint entryPoint, char* args) {
    struct Process* p = process_table_new(entryPoint, args, scheduler_current());
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
