#ifndef __SYSTEM_CACHE_CACHE__
#define __SYSTEM_CACHE_CACHE__

#include "system/lock/mutex.h"
#define SECTORS_PER_PAGE (PAGE_SIZE / SECTOR_SIZE)

struct CacheReference {
    unsigned long long sector;
    unsigned long long count;
    void* page;

    int refs;
    int atime;

    int evicting;
    int frozen;
    struct Mutex lock;

    struct CacheReference* next;
    struct CacheReference* prev;

    int index;
};

struct CacheReference* cache_get(unsigned long long sector);

void cache_freeze(struct CacheReference* ref);

void cache_repopulate(struct CacheReference* ref);

void cache_release(struct CacheReference* ref);

void cache_access(struct CacheReference* ref);

int cache_init(void);

int cache_sync(int force);

int cache_evict(int minPages);

void cache_flush(char* unused);

#endif
