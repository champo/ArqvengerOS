#ifndef __SYSTEM_PROCESS_SCHEDULER__
#define __SYSTEM_PROCESS_SCHEDULER__

#include "system/process/process.h"

void scheduler_add(struct Process* process);

void scheduler_do(void);

void scheduler_remove(struct Process* process);

struct Process* scheduler_current(void);

#endif
