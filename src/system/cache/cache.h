#ifndef __SYSTEM_CACHE_CACHE__
#define __SYSTEM_CACHE_CACHE__

#include "library/stddef.h"

int cache_read(unsigned long long sector, int count, void* buffer);

int cache_write(unsigned long long sector, int count, const void* buffer);

int cache_init(void);

int cache_sync(int force);

size_t cache_evict(size_t minPages);

void cache_flush(char* unused);

#endif
