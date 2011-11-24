#include "system/process/table.h"
#include "system/call.h"
#include "system/pinfo.h"
#include "library/div64.h"
#include "library/string.h"
#include "type.h"
#include "system/mm/page.h"
#include "system/scheduler.h"

/**
 *  Gets information of the processes.
 *
 *  @param data, an array of structures where the information will be returned.
 *  @param size, the lenght of the array.
 *  @return the quantity of structures read.
 */
int _pinfo(struct ProcessInfo* data, size_t size) {

    int pcount = 0;
    for (; pcount < size; pcount++) {
        struct Process* process = process_table_entry(pcount);
        if (process != NULL) {

            data[pcount].pid = process->pid;
            data[pcount].ppid = process->ppid;

            data[pcount].uid = process->uid;
            data[pcount].gid = process->gid;

            data[pcount].priority = process->schedule.priority;
            data[pcount].state = !process->schedule.done;

            data[pcount].cputime = uint64_div64(process->prev_cycles*100, scheduler_get_prev_sample_cycles());
            data[pcount].timeStart = process->timeStart;

            if (*process->args != 0) {
                int index = process->args - strchr(process->args, ' ') + 1;
                strncpy(data[pcount].name, process->args, index);
                data[pcount].name[index + 1] = '\0';
            } else {
                strcpy(data[pcount].name, "TOP SECRET");
            }
        } else {
            break;
        }
    }

    return pcount;
}

int _stacksize(void) {
    struct Process* process = scheduler_current();

    return process->mm.pagesInStack * PAGE_SIZE;
}
