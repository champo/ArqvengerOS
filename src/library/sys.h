#ifndef __LIBRARY_SYS__
#define __LIBRARY_SYS__

#include "type.h"

void yield(void);

pid_t wait(void);

void exit(void);

pid_t run(void(*entryPoint)(char*), char* args);

pid_t getpid(void);

pid_t getppid(void);

#endif
