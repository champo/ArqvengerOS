#include "system/scheduler.h"
#include "system/scheduler/choose_next.h"
#include "system/processQueue.h"
#include "system/sleepList.h"
#include "system/process/table.h"
#include "system/tick.h"
#include "type.h"

struct ProcessQueue scheduler_queue = {.first = NULL, .last = NULL};

struct Process* scheduler_curr = NULL;

union longlong {
    struct timeStampCounter { int low; int high; } tsc;
    unsigned long long val;
};

static unsigned long long cycles = 0;
static unsigned long long prev_cycles = 0;
static unsigned long long curr_cycles = 0;

SleepList scheduler_sleep_list = NULL;

static void update_cycles(void);

void scheduler_init(void) {
    scheduler_sleep_list = sleep_list_init();
}

void scheduler_add(struct Process* process) {
    process_queue_push(&scheduler_queue, process);
}

void scheduler_do(void) {

    if (scheduler_curr != NULL) {
        __asm__ __volatile ("mov %%ebp, %0":"=r"(scheduler_curr->mm.esp)::);

        update_cycles();
    }

    choose_next();

    if (scheduler_curr != NULL) {
        __asm__ __volatile__ ("mov %0, %%ebp"::"r"(scheduler_curr->mm.esp));
    }
}

void scheduler_remove(struct Process* process) {
    if (scheduler_curr != NULL && process->pid == scheduler_curr->pid) {
        scheduler_curr = NULL;
    }
    if (process->schedule.asleep) {
        sleep_list_remove(scheduler_sleep_list, process);
    }

    process_queue_remove(&scheduler_queue, process);
}

void scheduler_unblock(struct Process* process) {
    if (!process->schedule.done) {
        process_queue_push(&scheduler_queue, process);
    }
}

void scheduler_block(struct Process* process) {
    process_queue_remove(&scheduler_queue, process);
}

struct Process* scheduler_current(void) {
    if (scheduler_curr == NULL) {
        return NULL;
    }

    return scheduler_curr;
}

unsigned long long scheduler_get_cycles(void) {
    return cycles;
}

unsigned long long scheduler_get_prev_sample_cycles(void) {
    return prev_cycles;
}

void update_cycles(void) {

    union longlong a;
    unsigned long long newCycles;
    unsigned long long delta;

    __asm__ __volatile__ ("rdtsc; mov %%eax, %0; mov %%edx, %1": "=a"(a.tsc.low), "=d"(a.tsc.high));
    newCycles = a.val;

    delta = newCycles - cycles;
    scheduler_curr->cycles += delta;
    scheduler_curr->curr_cycles += delta;
    curr_cycles += delta;
    cycles = newCycles;
}

void scheduler_sleep(struct Process* process, int seconds) {
    sleep_list_add(scheduler_sleep_list, process, seconds*1000/MILLISECONDS_PER_TICK);
    process->schedule.asleep = 1;
}

void scheduler_tick(void) {
    sleep_list_update(scheduler_sleep_list);
}

void scheduler_restart_sample(void) {
    prev_cycles = curr_cycles;
    curr_cycles = 0;
}
