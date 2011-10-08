#ifndef __SYSTEM_SCHEDULER__
#define __SYSTEM_SCHEDULER__

#include "system/processQueue.h"
#include "system/process/process.h"

extern struct ProcessQueue scheduler_queue;

extern struct Process* scheduler_curr;

void scheduler_add(struct Process* process);

void scheduler_do(void);

void scheduler_remove(struct Process* process);

struct Process* scheduler_current(void);

unsigned long long scheduler_get_cycles(void);

unsigned long long scheduler_get_sample_cycles(void);

void scheduler_sleep(struct Process* process, int seconds);

void scheduler_unblock(struct Process* process);

void scheduler_block(struct Process* process);
#endif

