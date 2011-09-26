#include "system/scheduler/processQueue.h"

#define QUEUE_SIZE      256

struct QueueNode{
    struct QueueNode* next;    
    struct Process* process;
    int acumPriority;
};
 
struct ProcessQueue {
    struct QueueNode* first;
    struct QueueNode* last;
};
 

struct Process* process_queue_pop(struct ProcessQueue* queue) {
    
    if (queue->first == NULL) {
        return NULL;
    }
    
    struct Process* process;
    process = queue->first->process;
    
    struct QueueNode* aux = queue->first;
    if (queue->first == queue->last) {
        queue->first = queue->last = NULL;
    } else {
        queue->first = queue->first->next;
    }
    
    //TODO free(aux);
    return process;
}

void process_queue_push(struct ProcessQueue* queue, struct Process* process) {

    struct QueueNode* node = kalloc(sizeof(struct QueueNode));
    if (node == NULL) {
        //TODO error
        return;
    }
    node->process = process;
    node->acumPriority = 1;
    
    if (queue->first == NULL) {
        queue->first = queue->last = node;    
    } else {
        queue->last->next = node;
        queue->last = node;    
    }
    node->next = NULL;
}

struct Process* process_queue_peek(struct ProcessQueue* queue) {
    return queue->first->process;
}


void process_queue_remove(struct ProcessQueue* queue, struct Process* process) {
   
    struct QueueNode* prev;
    struct QueueNode* node;
    
    if (queue->first == NULL) {
        return;
    } 
    
    if (queue->first->process == process) {
        struct QueueNode* aux = queue->first;
        queue->first = queue->first->next; 
        //TODO free(aux);
    }
    

    node = queue->first->next;
    while (node != NULL && node->process != process) {
        prev = node;
        node = node->next;
    }

    if (node != NULL) {
        prev->next = node->next;
        //TODO free(node);
    }
}


