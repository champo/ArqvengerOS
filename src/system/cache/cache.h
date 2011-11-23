#ifndef __SYSTEM_CACHE_CACHE__
#define __SYSTEM_CACHE_CACHE__

#define CACHE_SIZE      (1 << 20)

struct Chunk {
    unsigned long long initialSector;
    void* buffer;
    int dirty;
    int accesses;
    int timeSinceLastAccess;
};

struct CacheTable {
    struct Chunk chunks[100];
    int memoryCached;
    int chunksNum;

};

int cache_read(unsigned long long sector, int count, void* buffer);

int cache_write(unsigned long long sector, int count, void* buffer);

#endif
