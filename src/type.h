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
    int uid;
    int gid;
    int cputime;
    time_t timeStart;
    char name[512];
};

#define TICKS_SAMPLE_SIZE 10
#define MILLISECONDS_PER_TICK 55

#endif
