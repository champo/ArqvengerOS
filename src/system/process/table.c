#include "system/process/table.h"
#include "system/scheduler.h"

#define PTABLE_SIZE 64

static struct Process *processTable[PTABLE_SIZE] = {0};

static struct Process* freeStructures = NULL;

static void process_table_remove(struct Process* process);

static struct Process* waitable_child(struct Process* process);

struct Process* process_table_new(EntryPoint entryPoint, char* args, struct Process* parent, int kernel, int terminal, int active) {

    size_t i;
    for (i = 0; i < PTABLE_SIZE; i++) {
        if (processTable[i] == NULL) {
            break;
        }
    }

    if (i == PTABLE_SIZE) {
        return NULL;
    }

    struct Process* p;
    if (freeStructures == NULL) {
        p = kalloc(sizeof(struct Process));
    } else {
        p = freeStructures;
        freeStructures = p->next;
    }

    processTable[i] = p;
    createProcess(p, entryPoint, parent, args, terminal, active);
    scheduler_add(p);

    return p;
}

void process_table_remove(struct Process* process) {

    for (size_t i = 0; i < PTABLE_SIZE; i++) {
        if (processTable[i] == process) {
            processTable[i] = NULL;
            break;
        }
    }

    destroyProcess(process);
    process->next = freeStructures;
    freeStructures = process;
}

void process_table_exit(struct Process* process) {

    if (process->firstChild) {

        struct Process* c = process->firstChild;
        struct Process* idle = processTable[0];

        do {
            // We asign it to the idle process
            c->parent = idle;
            c->ppid = idle->pid;
        } while (c->next && (c = c->next));

        c->next = idle->firstChild;
        idle->firstChild->prev = c;

        idle->firstChild = process->firstChild;
    }

    scheduler_remove(process);

    if (process->parent) {
        exitProcess(process);

        if (process->parent->schedule.inWait) {
            process->parent->schedule.status = StatusReady;
        }
    } else {
        process_table_remove(process);
    }
}

pid_t process_table_wait(struct Process* process) {

    if (process->firstChild != NULL) {

        struct Process* c;
        while ((c = waitable_child(process)) == NULL) {

            process->schedule.inWait = 1;
            process->schedule.status = StatusBlocked;

            scheduler_do();
        }

        process->schedule.inWait = 0;
        if (c->prev == NULL) {
            process->firstChild = c->next;
        } else {
            c->prev->next = c->next;
        }

        pid_t pid = c->pid;
        process_table_remove(c);

        return pid;
    }

    return -1;
}

struct Process* waitable_child(struct Process* process) {

    struct Process* c = process->firstChild;
    while (c != NULL) {

        if (c->schedule.done) {
            return c;
        }

        c = c->next;
    }

    return NULL;
}

struct Process* process_table_get(pid_t pid) {

    struct Process* c;
    for (size_t i = 0; i < PTABLE_SIZE; i++) {

        c = processTable[i];
        if (c != NULL && c->pid == pid) {
            return c;
        }
    }

    return NULL;
}

void process_table_kill(struct Process* process) {

    struct Process* c = process->firstChild;
    struct Process* next;
    while (c != NULL) {

        next = c->next;

        process_table_kill(c);
        process_table_remove(c);

        c = next;
    }

    process_table_exit(process);
}

