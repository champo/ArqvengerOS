#include "system/cache/cache.h"
#include "system/mm/allocator.h"
#include "drivers/ata.h"
#include "library/string.h"
#include "system/call.h"

#define CACHE_SIZE (1 << 20)
#define SECTORS_PER_PAGE (PAGE_SIZE / SECTOR_SIZE)
#define TABLE_ENTRIES 256

struct Chunk {
    unsigned long long initialSector;
    void* buffer;
    int dirty;
    int accesses;
    int lastWriteTime;
    int lastAccessTime;
};

static struct Chunk** table = NULL;

static struct Chunk* find_chunk(int index);

static int evict(void);

static void release_chunk(int tableIndex);

int cache_read(unsigned long long sector, int count, void* buffer) {

    char* buff = (char*) buffer;
    struct Chunk* chunk;
    int startBlock = sector / SECTORS_PER_PAGE;
    int endBlock = (sector + count) / SECTORS_PER_PAGE;
    int counter = 0;

    for (int block = startBlock; block <= endBlock; block++) {

        if ((chunk = find_chunk(block)) == NULL) {
            return -1;
        }

        chunk->accesses++;
        chunk->lastAccessTime = _time(NULL);

        if (block == startBlock) {

            int first = sector % SECTORS_PER_PAGE;
            int sectors = count;
            if (sectors + first > SECTORS_PER_PAGE) {
                sectors = SECTORS_PER_PAGE - first;
            }

            memcpy(buff, ((char*)chunk->buffer) + first * SECTOR_SIZE, SECTOR_SIZE * sectors);
            counter += sectors;

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

    for (int block = startBlock; block <= endBlock; block++) {

        if ((chunk = find_chunk(block)) == NULL) {
            return -1;
        }

        //TODO: Uncomment me when write-back is enabled
        //chunk->dirty = 1;
        chunk->accesses++;
        chunk->lastWriteTime = _time(NULL);
        chunk->lastAccessTime = chunk->lastWriteTime;

        if (block == startBlock) {

            int first = sector % SECTORS_PER_PAGE;
            int sectors = count;
            if (sectors + first > SECTORS_PER_PAGE) {
                sectors = SECTORS_PER_PAGE - first;
            }

            memcpy(((char*)chunk->buffer) + first * SECTOR_SIZE, buff, SECTOR_SIZE * sectors);
            counter += sectors;

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

    unsigned int sector = index * SECTORS_PER_PAGE;
    int firstEmpty = -1;

    for (int i = 0; i < TABLE_ENTRIES; i++) {

        if (table[i]) {
            if (table[i]->initialSector == sector) {
                return table[i];
            }
        } else if (firstEmpty == -1) {
            firstEmpty = i;
        }
    }

    if (firstEmpty == -1) {
        if (cache_evict(1) < 1) {
            log_error("Failed to evict enough pages... Something is gonna go awry.");
            return NULL;
        }

        for (int i = 0; i < TABLE_ENTRIES; i++) {

            if (!table[i]) {
                firstEmpty = i;
                break;
            }
        }
    }

    log_info("Loading page at sector %u into cache\n", sector);
    struct Chunk* entry = kalloc(sizeof(struct Chunk));
    entry->initialSector = sector;
    entry->buffer = allocPages(1);
    ata_read(sector, SECTORS_PER_PAGE, entry->buffer);
    entry->dirty = 0;
    entry->accesses = 0;
    entry->lastWriteTime = 0;
    entry->lastAccessTime = _time(NULL);
    table[firstEmpty] = entry;

    return entry;
}

int cache_init(void) {

    table = kalloc(sizeof(struct Chunk*) * TABLE_ENTRIES);
    if (table == NULL) {
        return -1;
    }

    for (int i = 0; i < TABLE_ENTRIES; i++) {
        table[i] = NULL;
    }

    return 0;
}

int cache_sync(int force) {

    int now = _time(NULL);
    for (int i = 0; i < TABLE_ENTRIES; i++) {

        struct Chunk* entry = table[i];
        if (entry) {

            // if lastWriteTime < now then more than one second has passed
            // One second dirty is a more than reasonable time (I think :P)
            if (entry->dirty && (force || entry->lastWriteTime < now)) {
                log_info("Flushing cache for sector %u entry\n", entry->initialSector);
                ata_write(entry->initialSector, SECTORS_PER_PAGE, entry->buffer);
                entry->dirty = 0;
            }
        }
    }

    return 0;
}

int cache_evict(int minPages) {


    // Ideally, we'd want to evict LRU pages.
    // Right we'll just evict the first minPages.

    int evicted = evict();
    if (evicted < minPages) {

        // If we didnt release enough, let's sync some pages
        // Hopefully it will free up enough memory
        cache_sync(1);
        evicted += evict();

        if (evicted < minPages) {
            // Now we start evicting agressively
            for (int i = 0; evicted < minPages && i < TABLE_ENTRIES; i++) {
                if (table[i]) {
                    release_chunk(i);
                    evicted++;
                }
            }
        }
    }

    return evicted;
}

int evict(void) {

    int now = _time(NULL);
    int evicted = 0;

    for (int i = 0; i < TABLE_ENTRIES; i++) {

        struct Chunk* entry = table[i];
        if (entry && !entry->dirty && entry->lastAccessTime < now) {
            release_chunk(i);
            evicted++;
        }
    }

    return evicted;
}

void release_chunk(int tableIndex) {

    struct Chunk* entry = table[tableIndex];
    log_info("Releasing chunk for sector %u\n", entry->initialSector);

    if (entry->dirty) {
        ata_write(entry->initialSector, SECTORS_PER_PAGE, entry->buffer);
    }

    freePages(entry->buffer, 1);
    kfree(entry);

    table[tableIndex] = NULL;
}

