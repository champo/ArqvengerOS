#include "system/call.h"
#include "system/process/table.h"
#include "library/stdio.h"

/**
 * Sets the gid and uid for a process.
 *
 * @param pid, the process id of the process to be changed.
 * @param gid, the new group id.
 * @param uid, the new user id.
 */
void _setProcessPersona(int pid, int uid, int gid) {

    struct Process* process = process_table_get(pid);
    if (process != NULL) {
        process->uid = uid;
        process->gid = gid; 
    }

    return;
}
