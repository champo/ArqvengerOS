#include "system/sleepList.h"
#include "system/scheduler.h"
#include "library/stdio.h"

struct DeltaQueue {
    struct Node*   first;    
};

struct Node {
    struct Node* next;
    struct Process* process;
    int delta;
};

static struct Node* freeNodes = NULL;

static void free_node(struct Node* node);

static struct Node* alloc_node(void);

static void update_list(struct Node* startNode, int value);

struct Node* alloc_node(void) {

    struct Node* node;
    if (freeNodes == NULL) {
        node = kalloc(sizeof(struct Node));
        if (node == NULL) {
            return NULL;
        }
    } else {
        node = freeNodes;
        freeNodes = node->next;
    }

    return node;
}

void free_node(struct Node* node) {
    node->next = freeNodes;
    freeNodes = node;
}


SleepList sleep_list_init(void) {
    struct DeltaQueue* list = kalloc(sizeof(struct Node));
    list->first = NULL;
    return list;
}

void sleep_list_add(SleepList list, struct Process* process, int value) {
    
    struct Node* node = alloc_node();
    struct Node* curr;
    struct Node* prev;
    node->process = process;
    
    if (list->first == NULL || list->first >= value) {  
        struct Node* aux = list->first;
        
        list->first = node;
        list->first->delta = value;
        list->first->next = aux;
        
        update_list(list->first->next, value);

    } else {
        curr = list->first;
        
        while (curr != NULL && (value - curr->delta) <= 0) {
            value -= curr->delta;
            prev = curr;
            curr = curr->next;
        }

        prev->next = node;
        node->next = curr;
        node->delta = value;
        
        update_list(curr->next, value);
    }
}

void update_list(struct Node* curr, int value) {

    while (curr != NULL) {
        curr->delta -= value;
        curr = curr->next;
    }
}


void sleep_list_update(SleepList list) {

    if (list->first != NULL) {
        list->first->delta--;
    } 

    while (list->first != NULL && list->first->delta == 0) {
        //struct Node* aux = list->first;
        list->first->process->schedule.status = StatusReady;
        list->first = list->first->next;
        //TODO free_node(aux);
    }
}

void sleep_list_free(SleepList list) {
    //TODO
}

