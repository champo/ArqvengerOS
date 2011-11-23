#ifndef __SYSTEM_CACHE_CACHE__
#define __SYSTEM_CACHE_CACHE__

int cache_read(unsigned long long sector, int count, void* buffer);

int cache_write(unsigned long long sector, int count, void* buffer);

#endif
