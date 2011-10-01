#ifndef __SYSTEM_PROCESS_QUEUE_
#define __SYSTEM_PROCESS_QUEUE_

#include "system/process/process.h"
#include "type.h"

struct QueueNode{
    struct QueueNode* next;
    struct Process* process;
    int acumPriority;
};

struct ProcessQueue {
    struct QueueNode* first;
    struct QueueNode* last;
};

void process_queue_push(struct ProcessQueue* queue, struct Process* process);

void process_queue_remove(struct ProcessQueue* queue, struct Process* process);

struct Process* process_queue_pop(struct ProcessQueue* queue);
#endif
