#ifndef __DRIVERS_TTY__
#define __DRIVERS_TTY__

#include "type.h"

size_t writeScreen(const void* buf, size_t length);

void initScreen(void);

#endif
