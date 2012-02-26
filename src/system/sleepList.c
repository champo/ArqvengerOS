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

static void update_list(struct Node* startNode, int ticks);

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
    struct DeltaQueue* list = kalloc(sizeof(struct DeltaQueue));
    list->first = NULL;
    return list;
}

void sleep_list_add(SleepList list, struct Process* process, int ticks) {

    struct Node* node = alloc_node();
    struct Node* curr;
    struct Node* prev;
    node->process = process;

    if (list->first == NULL || list->first->delta >= ticks) {
        struct Node* aux = list->first;

        list->first = node;
        list->first->delta = ticks;
        list->first->next = aux;

        update_list(list->first->next, ticks);

    } else {
        curr = list->first;

        while (curr != NULL && curr->delta <= ticks) {
            ticks -= curr->delta;
            prev = curr;
            curr = curr->next;
        }

        prev->next = node;
        node->next = curr;
        node->delta = ticks;

        update_list(curr->next, ticks);
    }
}

void update_list(struct Node* curr, int ticks) {

    if (curr != NULL) {
        curr->delta -= ticks;
    }
}


void sleep_list_update(SleepList list) {

    if (list->first != NULL) {
        list->first->delta--;
    }

    while (list->first != NULL && list->first->delta == 0) {
        struct Node* aux = list->first;

        list->first->process->schedule.status = StatusReady;
        list->first->process->schedule.asleep = 0;
        scheduler_unblock(list->first->process);

        list->first = list->first->next;
        free_node(aux);
    }
}

void sleep_list_remove(SleepList list, struct Process* process) {
    struct Node* curr = list->first;
    struct Node* prev = NULL;

    while (curr != NULL && curr->process->pid != process->pid) {
        prev = curr;
        curr = curr->next;
    }

    if (curr != NULL) {

        if (prev) {
            prev->next = curr->next;
        } else {
            list->first = curr->next;
        }

        if (curr->next) {
            curr->next->delta += curr->delta;
        }

        free_node(curr);
    }
}

void sleep_list_free(SleepList list) {
    //TODO
}

