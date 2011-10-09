#ifndef __SYSTEM_PINFO__
#define __SYSTEM_PINFO__

struct ProcessInfo {
    pid_t pid;
    pid_t ppid;
    int priority;
    int state;
    int uid;
    int gid;
    int cputime;
    time_t timeStart;
    char name[512];
};

#endif
