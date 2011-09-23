#include "system/process/table.h"
#include "system/process/scheduler.h"

#define PTABLE_SIZE 64

struct Process *processTable[PTABLE_SIZE] = {0};

static void process_table_remove(struct Process* process);

static struct Process* waitable_child(struct Process* process);

struct Process* process_table_new(EntryPoint entryPoint, char* args, struct Process* parent) {

    struct Process* p = kalloc(sizeof(struct Process));

    for (size_t i = 0; i < PTABLE_SIZE; i++) {
        if (processTable[i] == NULL) {
            processTable[i] = p;
            break;
        }
    }

    createProcess(p, entryPoint, parent, args);
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
}

void process_table_exit(struct Process* process) {

    if (process->children) {

        for (size_t i = 0; i < PTABLE_SIZE; i++) {

            if (processTable[i] != NULL && processTable[i]->ppid == process->pid) {
                // We asign it to the idle process
                processTable[i]->parent = processTable[0];
                processTable[i]->ppid = 1;
            }
        }
    }

    scheduler_remove(process);

    if (process->parent) {
        exitProcess(process);

        if (process->parent->schedule.inWait) {
            process->parent->schedule.status = StatusReady;
        }
    } else {
        process_table_remove(process);
        destroyProcess(process);
        //TODO: kfree?
    }
}

int process_table_wait(struct Process* process) {

    if (process->children != 0) {

        struct Process* c;
        while ((c = waitable_child(process)) == NULL) {

            process->schedule.inWait = 1;
            process->schedule.status = StatusBlocked;

            scheduler_do();
        }

        process->schedule.inWait = 0;

        if (c != NULL) {
            process_table_remove(c);
            destroyProcess(c);
            process->children--;
        }
    }
}

struct Process* waitable_child(struct Process* process) {

    struct Process* c;
    for (size_t i = 0; i < PTABLE_SIZE; i++) {

        c = processTable[i];
        if (c != NULL && c->ppid == process->pid) {
            if (c->schedule.done) {
                return c;
            }
        }
    }

    return NULL;
}

