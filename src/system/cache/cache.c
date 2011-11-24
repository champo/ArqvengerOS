#include "system/cache/cache.h"
#include "system/mm/allocator.h"
#include "drivers/ata.h"
#include "library/string.h"

#define CACHE_SIZE (1 << 20)
#define SECTORS_PER_PAGE (PAGE_SIZE / SECTOR_SIZE)

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

static struct CacheTable table;

static struct Chunk* find_chunk(int index);

int cache_read(unsigned long long sector, int count, void* buffer) {

    char* buff = (char*) buffer;
    struct Chunk* chunk;
    int startBlock = sector / SECTORS_PER_PAGE;
    int endBlock = (sector + count) / SECTORS_PER_PAGE;
    int counter = 0;

    for (int block = startBlock; block < endBlock; block++) {

        if ((chunk = find_chunk(block)) == NULL) {
            return -1;
        }

        chunk->accesses++;

        if (block == startBlock) {

            int first = sector % SECTORS_PER_PAGE;
            int last = count > SECTORS_PER_PAGE ? SECTORS_PER_PAGE : count;

            memcpy(buff, ((char*)chunk->buffer) + first * SECTOR_SIZE, SECTOR_SIZE * (last - first));
            counter += (last - first);

        } else if (block == endBlock) {

            int last = (sector + count) % SECTORS_PER_PAGE;
            memcpy(buff + counter * SECTOR_SIZE, chunk->buffer, SECTOR_SIZE * last);
            counter += last;
        } else {

            memcpy(buff + counter * SECTOR_SIZE, chunk->buffer, SECTOR_SIZE * SECTORS_PER_PAGE);
            counter += SECTORS_PER_PAGE;
        }
    }

    return 0;
}


int cache_write(unsigned long long sector, int count, void* buffer) {

    char* buff = (char*) buffer;
    struct Chunk* chunk;
    int startBlock = sector / SECTORS_PER_PAGE;
    int endBlock = (sector + count) / SECTORS_PER_PAGE;
    int counter = 0;

    for (int block = startBlock; block < endBlock; block++) {

        if ((chunk = find_chunk(block)) == NULL) {
            return -1;
        }

        chunk->dirty = 1;
        chunk->accesses++;

        if (block == startBlock) {

            int first = sector % SECTORS_PER_PAGE;
            int last = count > SECTORS_PER_PAGE ? SECTORS_PER_PAGE : count;

            memcpy(((char*)chunk->buffer) + first * SECTOR_SIZE, buff, SECTOR_SIZE * (last - first));
            counter += (last - first);

        } else if (block == endBlock) {

            int last = (sector + count) % SECTORS_PER_PAGE;
            memcpy(chunk->buffer, buff + counter * SECTOR_SIZE, SECTOR_SIZE * last);
            counter += last;
        } else {

            memcpy(chunk->buffer, buff + counter * SECTOR_SIZE, SECTOR_SIZE * SECTORS_PER_PAGE);
            counter += SECTORS_PER_PAGE;
        }

    }

    //write-trough
    ata_write(sector, count, buffer);

    return 0;
}

struct Chunk* find_chunk(int index) {
    return NULL;
}

