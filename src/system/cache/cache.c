#include "system/cache/cache.h"
#include "system/mm/allocator.h"
#include "system/kprintf.h"
#include "drivers/ata.h"
#include "library/string.h"
#include "system/call.h"
#include "system/common.h"
#include "library/sys.h"

#define SECTORS_PER_PAGE (PAGE_SIZE / SECTOR_SIZE)
#define TABLE_ENTRIES 8

struct Chunk {
    unsigned long long initialSector;
    void* buffer;
    int dirty;
    int accesses;
    int firstWriteTime;
    int lastAccessTime;
    struct Chunk* next;
    struct Chunk* prev;

    int index;
};


struct LRUList {
    struct Chunk* first;
    struct Chunk* last;
};


static struct Chunk** table = NULL;

static struct LRUList cache_list = {.first = NULL, .last = NULL};

static struct Chunk* find_chunk(int index);

static int evict(int minPages, int force);

static void release_chunk(int tableIndex);

static void cache_list_add(struct LRUList* list, struct Chunk* chunk);

static void cache_list_remove(struct LRUList* list, struct Chunk* chunk);

static void mark_access(struct Chunk* chunk);

/**
 *  Cache flush process entry point.
 *
 */
void cache_flush(char* unused) {

    while (1) {
        disableInterrupts();
        cache_sync(0);
        enableInterrupts();
        sleep(1);
    }
}

void cache_list_add(struct LRUList* list, struct Chunk* chunk) {

    //We add always at the beginning
    struct Chunk* aux = list->first;
    list->first = chunk;

    chunk->next = aux;
    chunk->prev = NULL;

    if (aux != NULL) {
        aux->prev = list->first;
    }

    if (list->last == NULL) {
        list->last = list->first;
    }

    return;
}

void cache_list_remove(struct LRUList* list, struct Chunk* chunk) {

    // We don't have to look for the chunk in the list as the chunk itself is a node.
    struct Chunk* next = chunk->next;
    struct Chunk* previous = chunk->prev;

    chunk->prev = NULL;
    chunk->next = NULL;

    if (previous != NULL) {
        previous->next = next;
    } else {
        list->first = next;
    }

    if (next != NULL) {
        next->prev = previous;
    } else {
        list->last = previous;
    }
}

void mark_access(struct Chunk* chunk) {

    chunk->accesses++;
    chunk->lastAccessTime = _time(NULL);

    cache_list_remove(&cache_list, chunk);
    cache_list_add(&cache_list, chunk);
}

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

        mark_access(chunk);

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
        mark_access(chunk);

        if (!chunk->dirty) {
            chunk->firstWriteTime = _time(NULL);
        }

        chunk->dirty = 1;

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
    entry->firstWriteTime = 0;
    entry->lastAccessTime = _time(NULL);
    entry->index = firstEmpty;
    table[firstEmpty] = entry;

    cache_list_add(&cache_list, entry);

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
            if (entry->dirty && (force || entry->firstWriteTime < now)) {
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

    int evicted = evict(minPages, 0);
    if (evicted < minPages) {

        // If we didnt release enough, let's sync some pages
        // Hopefully it will free up enough memory
        log_debug("Force a sync & try to evict more pages\n");
        cache_sync(1);
        evicted += evict(minPages - evicted, 0);

        if (evicted < minPages) {
            // Now we start evicting agressively
            log_debug("Going agressive with the eviction...\n");
            evicted += evict(minPages - evicted, 1);
        }
    }

    return evicted;
}

int evict(int minPages, int force) {

    int now = _time(NULL);
    int evicted = 0;

    struct Chunk* entry = cache_list.last;
    while (entry && evicted < minPages) {

        if (!entry->dirty && (entry->lastAccessTime < now || force)) {
            release_chunk(entry->index);
            evicted++;
        }

        entry = entry->prev;
    }

    return evicted;
}

void release_chunk(int tableIndex) {

    struct Chunk* entry = table[tableIndex];
    log_info("Releasing chunk for sector %u\n", entry->initialSector);

    if (entry->dirty) {
        ata_write(entry->initialSector, SECTORS_PER_PAGE, entry->buffer);
    }

    cache_list_remove(&cache_list, entry);

    freePages(entry->buffer, 1);
    kfree(entry);

    table[tableIndex] = NULL;
}

