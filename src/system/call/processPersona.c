#include "system/call.h"
#include "system/process/table.h"
#include "library/stdio.h"

void _setProcessPersona(int pid, int uid, int gid) {

    struct Process* process = process_table_get(pid);
    if (process != NULL) {
        process->uid = uid;
        process->gid = gid; 
    }

    return;
}

void _getProcessPersona(int pid, int* uid, int* gid) {

    struct Process* process = process_table_get(pid);
    if (process != NULL) {
        *uid = process->uid;
        *gid = process->gid;
    }

    return;
}
