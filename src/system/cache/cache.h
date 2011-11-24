#ifndef __SYSTEM_CACHE_CACHE__
#define __SYSTEM_CACHE_CACHE__

int cache_read(unsigned long long sector, int count, void* buffer);

int cache_write(unsigned long long sector, int count, void* buffer);

int cache_init(void);

int cache_sync(int force);

int cache_evict(int minPages);

void cache_flush(char* unused);

#endif
