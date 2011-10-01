#include "system/scheduler/choose_next.h"
#include "system/scheduler.h"
#include "system/processQueue.h"
#include "type.h"


void choose_next(void) {

    if (scheduler_curr != NULL && scheduler_curr->schedule.status == StatusRunning) {
        scheduler_curr->schedule.status = StatusReady;
    }
    struct Process* process;

    do {
        process = process_queue_pop(&scheduler_queue);        
        if (process == NULL) {
            scheduler_curr = NULL;
            return;
        }
        process_queue_push(&scheduler_queue, process);
    } while (process->schedule.status == StatusBlocked);

    scheduler_curr = process;

    scheduler_curr->schedule.status = StatusRunning;
}

