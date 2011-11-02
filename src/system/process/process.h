#ifndef __SYSTEM_PROCESS_PROCESS__
#define __SYSTEM_PROCESS_PROCESS__

#include "system/mm.h"
#include "type.h"
#include "system/fs/fd.h"

#define NO_TERMINAL -1
#define MAX_OPEN_FILES 10
#define KERNEL_STACK_PAGES 10

typedef void (*EntryPoint)(char*);

struct ProcessMemory {
    void* esp;
    void* stackStart;
    int pagesInStack;

    void* heap;
    void* mallocContext;
    int pagesInHeap;

    void* esp0;
    void* kernelStack;
    void* kernelStackStart;
    int pagesInKernelStack;
};

enum ProcessStatus {
    StatusRunning,
    StatusReady,
    StatusBlocked
};

struct ProcessSchedule {
    enum ProcessStatus status;
    unsigned int priority:2;
    unsigned int inWait:1;
    unsigned int ioWait:1;
    unsigned int asleep:1;
    unsigned int done:1;
};

struct Process {
    int pid;
    int kernel;

    int ppid;
    struct Process* parent;

    EntryPoint entryPoint;
    char args[512];

    int terminal;
    int active;

    struct Process* firstChild;

    struct Process* prev;
    struct Process* next;

    struct ProcessSchedule schedule;
    struct ProcessMemory mm;

    unsigned long long cycles;
    unsigned long long curr_cycles;
    unsigned long long prev_cycles;
    time_t timeStart;

    int uid;
    int gid;

    struct FileDescriptor fdTable[MAX_OPEN_FILES];

    char* cwd;
};

void createProcess(struct Process* process, EntryPoint entryPoint, struct Process* parent, char* args, int terminal, int kernel);

void destroyProcess(struct Process* process);

void exitProcess(struct Process* process);

#endif
