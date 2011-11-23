#ifndef __SYSTEM_PROCESS_PROCESS__
#define __SYSTEM_PROCESS_PROCESS__

#include "type.h"
#include "system/fs/fd.h"
#include "system/mm/page.h"

#define ARGV_SIZE 256

#define NO_TERMINAL -1
#define MAX_OPEN_FILES 10
#define KERNEL_STACK_PAGES 10

#define STACK_TOP_MAPPING (3 * 1024 * 1024 * 1024u)

typedef void (*EntryPoint)(char[ARGV_SIZE]);

struct ProcessMemory {
    void* esp;
    int pagesInStack;

    void* mallocContext;
    int pagesInHeap;

    void* esp0;
    void* kernelStack;
    int pagesInKernelStack;

    struct Pages* reservedPages;

    struct PageDirectory* directory;
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
    char args[ARGV_SIZE];

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
