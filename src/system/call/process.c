#include "system/call.h"
#include "system/process/table.h"
#include "system/process/scheduler.h"

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
    struct Process* p = scheduler_current();
    process_table_exit(p);
}

void _wait(void) {
    //TODO: Do wait
}

