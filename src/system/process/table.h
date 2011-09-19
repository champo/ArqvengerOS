#ifndef __SYSTEM_PROCESS_TABLE__
#define __SYSTEM_PROCESS_TABLE__

#include "system/mm.h"

struct ProcessMemory {
    void* esp;
    void* stackStart;
    int pagesInStack;
}

struct Process {
    int pid;
    struct ProcessMemory mm;
};

#endif
