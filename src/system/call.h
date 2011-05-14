#ifndef _system_call_header_
#define _system_call_header_

#include "type.h"

size_t _write(int fd, const void* buf, size_t length);

size_t _read(int fd, void* buf, size_t length);

int _ioctl(int fd, int cmd, void* argp);

#endif
