#ifndef _type_header
#define _type_header

typedef unsigned int size_t;

typedef unsigned char byte;
typedef short word;
typedef int dword;

typedef unsigned int time_t;

typedef int pid_t;

struct ProcessInfo {
    pid_t pid;
    pid_t ppid;
    int priority;
    int state;
    //startDate;
    //uid;
    //gid;
    int cputime;
    time_t timeStart;
};

#endif
