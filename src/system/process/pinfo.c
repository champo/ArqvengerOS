#include "system/process/pinfo.h"
#include "system/process/table.h"
#include "system/mm.h"
#include "system/call.h"

int _pinfo(struct ProcessInfo* data, size_t size) {

    int pcount = 0;
    for (int pid = 0; pid < size; pid++) {
        struct Process* process = process_table_get(pid);
        if (process != NULL ) {
    
            data[pcount].pid = process->pid;
            data[pcount].ppid = process->ppid;
            data[pcount].priority = process->schedule.priority;
            data[pcount].state = !process->schedule.done;
            //data[pcount].cputime = process->cputime;
            pcount++;
        }
    }
    
    return pcount;
}
