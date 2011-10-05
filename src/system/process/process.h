#ifndef __SYSTEM_PROCESS_PROCESS__
#define __SYSTEM_PROCESS_PROCESS__

#include "system/mm.h"
#include "type.h"

#define NO_TERMINAL -1

typedef void (*EntryPoint)(char*);

struct ProcessMemory {
    void* esp;
    void* stackStart;
    int pagesInStack;
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
    unsigned int done:1;
};

struct Process {
    int pid;

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
    time_t timeStart;

    int uid;
    int gid;
};

void createProcess(struct Process* process, EntryPoint entryPoint, struct Process* parent, char* args, int terminal);

void destroyProcess(struct Process* process);

void exitProcess(struct Process* process);

#endif
