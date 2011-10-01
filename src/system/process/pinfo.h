#ifndef __SYSTEM_PROCESS_PINFO
#define __SYSTEM_PROCESS_PINFO

#include "type.h"

struct ProcessInfo {
    pid_t pid;
    pid_t ppid;
    int priority;
    int state;
    //startDate;
    //uid;
    //gid;
};


int _pinfo(struct ProcessInfo* data, size_t size);

#endif
