#include "system/scheduler/scheduler.h"
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

struct Process* process_queue_pop(struct ProcessQueue* queue);

void process_queue_push(struct ProcessQueue* queue, struct Process* process);

struct Process* process_queue_peek(struct ProcessQueue* queue);

void process_queue_remove(struct ProcessQueue* queue, struct Process* process);



