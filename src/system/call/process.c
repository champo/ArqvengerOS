#include "system/call.h"
#include "system/process/table.h"
#include "system/scheduler.h"

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

