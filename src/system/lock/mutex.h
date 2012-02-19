#ifndef __SYSTEM_LOCK_MUTEX__
#define __SYSTEM_LOCK_MUTEX__

#include "system/processQueue.h"
#include "system/process/process.h"

struct Mutex {
    struct ProcessQueue queue;

    int levels;
    struct Process* owner;
};

void mutex_init(struct Mutex* mutex);

void mutex_lock(struct Mutex* mutex);

void mutex_release(struct Mutex* mutex);

#endif
