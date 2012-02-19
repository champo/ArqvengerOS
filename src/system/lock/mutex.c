#include "system/lock/mutex.h"
#include "system/process/table.h"
#include "system/scheduler.h"

static void wake_all(struct ProcessQueue* queue);

static void wait(struct ProcessQueue* queue, struct Process* process);

void mutex_init(struct Mutex* mutex) {
    mutex->queue.first = NULL;
    mutex->queue.last = NULL;

    mutex->owner = NULL;
    mutex->levels = 0;
}

void mutex_lock(struct Mutex* mutex) {

    struct Process* me = scheduler_current();
    if (me == NULL) {
        return;
    }

    if (mutex->owner == me) {
        mutex->levels++;
    } else {

        while (mutex->owner != NULL) {
            wait(&mutex->queue, me);
        }
        me->schedule.inWait = 0;

        mutex->owner = me;
        mutex->levels++;
    }
}

void mutex_release(struct Mutex* mutex) {

    if (mutex->owner != scheduler_current() || mutex->levels == 0) {
        // We'll just ignore release calls that have no effect
        return;
    }

    mutex->levels--;
    if (mutex->levels == 0) {
        mutex->owner = NULL;
        wake_all(&mutex->queue);
    }
}

void wake_all(struct ProcessQueue* queue) {
    struct Process* process = process_queue_pop(queue);

    while (process != NULL) {
        process_table_unblock(process);
        process = process_queue_pop(queue);
    }
}

void wait(struct ProcessQueue* queue, struct Process* process) {

    process_queue_push(queue, process);

    process->schedule.inWait = 1;
    process_table_block(process);

    scheduler_do();
}

