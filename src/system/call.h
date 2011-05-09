#ifndef _system_call_header_
#define _system_call_header_

#include "type.h"

size_t write(int fd, const void* buf, size_t length);

size_t read(int fd, void* buf, size_t length);

#endif
