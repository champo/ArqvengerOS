#ifndef _drivers_keyboard_header
#define _drivers_keyboard_header

#include "type.h"

void int09();

size_t read(int fd, void* buffer, size_t count);

#endif
