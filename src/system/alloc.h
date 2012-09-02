#ifndef __SYSTEM_ALLOC__
#define __SYSTEM_ALLOC__

#include "library/stddef.h"

void* kalloc(size_t size);

void kfree(void* data);

#endif
