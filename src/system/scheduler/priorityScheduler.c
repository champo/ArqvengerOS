#include "system/scheduler/choose_next.h"
#include "system/scheduler.h"
#include "system/processQueue.h"
#include "type.h"

static struct QueueNode* top_priority = NULL;

static void updateAcumPriorities(void);

void choose_next(void) {

    if (scheduler_curr != NULL && scheduler_curr->schedule.status == StatusRunning) {
        scheduler_curr->schedule.status = StatusReady;
    }

    updateAcumPriorities();
    if (top_priority != NULL) {
        scheduler_curr = top_priority->process;
        top_priority->acumPriority = 1;

        scheduler_curr->schedule.status = StatusRunning;
    }
}

void updateAcumPriorities(void) {

    struct QueueNode* node = scheduler_queue.first;
    top_priority = scheduler_queue.first;

    while (node != NULL) {

        if (node->process->schedule.status != StatusBlocked) {

            node->acumPriority += (node->process->schedule.priority + 1);
            if (node->acumPriority > top_priority->acumPriority) {
                top_priority = node;
            }
        }

        node = node->next;
    }
}


