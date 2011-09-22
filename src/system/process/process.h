#ifndef __SYSTEM_PROCESS_PROCESS__
#define __SYSTEM_PROCESS_PROCESS__

#include "system/mm.h"

struct ProcessMemory {
    void* esp;
    void* stackStart;
    int pagesInStack;
};

struct Process {
    int pid;
    struct ProcessMemory mm;
};

void createProcess(struct Process* process, void* entrypoint);

void destroyProcess(struct Process* process);

#endif
