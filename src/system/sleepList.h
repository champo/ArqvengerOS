#ifndef __SYSTEM_SLEEPLIST__
#define __SYSTEM_SLEEPLIST__

#include "system/process/process.h"

typedef struct DeltaQueue* SleepList;

SleepList sleep_list_init(void);

void sleep_list_add(SleepList list, struct Process* process, int value);

void sleep_list_update(SleepList list);

void sleep_list_free(SleepList list);
#endif
