#ifndef _common_header_
#define _common_header_

#include "type.h"

#define halt() __asm__ volatile ("hlt")

#define enableInterrupts() __asm__ volatile ("sti")

#define disableInterrupts() __asm__ volatile ("cli")

int _isIF();

#endif
