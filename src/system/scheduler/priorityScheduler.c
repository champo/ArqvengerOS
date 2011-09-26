#include "system/scheduler/scheduler.h"
#include "system/processQueue.h"
#include "type.h"


static struct ProcessQueue queue = {.first = NULL, .last = NULL};

static struct QueueNode* current = NULL;

static struct QueueNode* topPriority = NULL;

static void choose_next(void);

static void updateAcumPriorities(void);

void scheduler_add(struct Process* process) {
    process_queue_push(&queue, process);
}

void scheduler_do(void) {
    
    if (current != NULL) {
        __asm__ __volatile ("mov %%ebp, %0":"=r"(current->process->mm.esp)::);
    }

    choose_next();

    if (current != NULL) {
        __asm__ __volatile__ ("mov %0, %%ebp"::"r"(current->process->mm.esp));
    }
}

void choose_next(void) {
    
    if (current != NULL && current->process->schedule.status == StatusRunning) {
        current->process->schedule.status = StatusReady;
    }

    updateAcumPriorities();
    if (topPriority != NULL) {
        current = topPriority;
        topPriority->acumPriority = 1;
       
        current->process->schedule.status = StatusRunning;    
    }
}

void updateAcumPriorities(void) {
    topPriority = queue.first; 
    struct QueueNode* node = queue.first;
    while (node != NULL) {
        if (node->process->schedule.status != StatusBlocked) { 
            node->acumPriority += (node->process->schedule.priority + 1);
            if (node->acumPriority > topPriority->acumPriority) {
                topPriority = node;
            }
        }
        node = node->next;

    }
        writeScreen("   ",3);
}


void scheduler_remove(struct Process* process) {
    process_queue_remove(&queue, process);
}


struct Process* scheduler_current(void) {    
    if (current == NULL) {
        return NULL;
    }

    return current->process;

}
