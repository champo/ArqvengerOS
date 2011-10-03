#include "system/scheduler.h"
#include "system/scheduler/choose_next.h"
#include "system/processQueue.h"
#include "type.h"

struct ProcessQueue scheduler_queue = {.first = NULL, .last = NULL};

struct Process* scheduler_curr = NULL;

static union longlong { 
    struct timeStampCounte { int low; int high; } tsc; 
    unsigned long long val; 
};

static unsigned long long cycles = 0;

static void update_cycles();

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


void update_cycles() {
    int low,high;
    union longlong a;
    unsigned long long newCycles;
    
    __asm__ __volatile__ ("rdtsc; mov %%eax, %0; mov %%edx, %1": "=a"(low), "=d"(high)); 
    
    a.tsc.low = low;
    a.tsc.high = high;
    newCycles = a.val;
    
    scheduler_curr->cycles += (newCycles - cycles);
    cycles = newCycles;
}



