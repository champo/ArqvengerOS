#ifndef _system_call_header_
#define _system_call_header_

#include "type.h"

size_t _write(int fd, const void* buf, size_t length);

size_t _read(int fd, void* buf, size_t length);

int _ioctl(int fd, int cmd, void* argp);

time_t _time(time_t *tp);

pid_t _getpid(void);

pid_t _getppid(void);

pid_t _run(void(*EntryPoint)(char*), char* args);

void _exit(void);

pid_t _wait(void);

int _pinfo(struct ProcessInfo* data, size_t size);

void _sleep(int seconds);

int _nice(int priority);

#endif
