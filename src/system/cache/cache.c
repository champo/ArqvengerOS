#include "system/cache/cache.h"
#include "system/mm/allocator.h"
#include "system/mm/pagination.h"
#include "system/kprintf.h"
#include "drivers/ata.h"
#include "system/common.h"
#include "library/sys.h"
#include "library/call.h"
#include "system/call/codes.h"

#define SECTORS_PER_PAGE (PAGE_SIZE / SECTOR_SIZE)
#define TABLE_ENTRIES 32

struct LRUList {
    struct CacheReference* first;
    struct CacheReference* last;
};

static struct Mutex tableLock;

static struct CacheReference** table = NULL;

static struct LRUList cache_list = {.first = NULL, .last = NULL};

static struct CacheReference* freezer;

static struct CacheReference* evicted = NULL;

static struct CacheReference* find_chunk(unsigned long long sector);

static int evict(int minPages, int force);

static int release_chunk(int tableIndex);

static void* reserve_page(void);

static void cache_list_add(struct LRUList* list, struct CacheReference* chunk);

static void cache_list_remove(struct LRUList* list, struct CacheReference* chunk);

/**
 *  Cache flush process entry point.
 */
void cache_flush(char* unused) {

    while (1) {
        system_call(_SYS_CACHE_SYNC, 0, 0, 0);
        sleep(1);
    }
}

void cache_list_add(struct LRUList* list, struct CacheReference* chunk) {

    //We add always at the beginning
    struct CacheReference* aux = list->first;
    list->first = chunk;

    chunk->next = aux;
    chunk->prev = NULL;

    if (aux != NULL) {
        aux->prev = list->first;
    }

    if (list->last == NULL) {
        list->last = list->first;
    }
}

void cache_list_remove(struct LRUList* list, struct CacheReference* chunk) {

    // We don't have to look for the chunk in the list as the chunk itself is a node.
    struct CacheReference* next = chunk->next;
    struct CacheReference* previous = chunk->prev;

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

struct CacheReference* find_chunk(unsigned long long sector) {

    mutex_lock(&tableLock);
    int firstEmpty = -1;
    for (int i = 0; i < TABLE_ENTRIES; i++) {

        if (table[i]) {
            if (table[i]->sector == sector) {
                mutex_release(&tableLock);
                return table[i];
            }
        } else if (firstEmpty == -1) {
            firstEmpty = i;
        }
    }

    struct CacheReference* entry = evicted;
    while (entry) {

        if (entry->sector == sector) {
            mutex_release(&tableLock);
            return entry;
        }

        entry = entry->next;
    }

    if (firstEmpty == -1) {

        if (cache_evict(1) < 1) {
            log_error("Failed to evict enough pages... Something is gonna go awry.");
            mutex_release(&tableLock);
            return NULL;
        }

        for (int i = 0; i < TABLE_ENTRIES; i++) {
            if (!table[i]) {
                firstEmpty = i;
                break;
            }
        }
    }

    log_info("Loading page at sector %u into cache at %d\n", sector, firstEmpty);

    entry = kalloc(sizeof(struct CacheReference));
    entry->sector = sector;
    entry->count = SECTORS_PER_PAGE;
    entry->page = reserve_page();

    entry->refs = 0;
    entry->atime = _time(NULL);

    entry->evicting = 0;
    entry->frozen = 0;
    mutex_init(&entry->lock);

    entry->index = firstEmpty;

    mutex_lock(&entry->lock);

    cache_list_add(&cache_list, entry);
    table[firstEmpty] = entry;
    mutex_release(&tableLock);

    ata_read(sector, SECTORS_PER_PAGE, entry->page);
    mutex_release(&entry->lock);

    return entry;
}

int cache_init(void) {

    mutex_init(&tableLock);

    table = kalloc(sizeof(struct CacheReference*) * TABLE_ENTRIES);
    if (table == NULL) {
        return -1;
    }

    for (int i = 0; i < TABLE_ENTRIES; i++) {
        table[i] = NULL;
    }

    return 0;
}

int cache_sync(int force) {

    mutex_lock(&tableLock);
    for (int i = 0; i < TABLE_ENTRIES; i++) {

        struct CacheReference* entry = table[i];
        if (entry) {

            //FIXME: As is, every call to sync writes *every* dirty page
            if (mm_pagination_is_dirty(entry->page)) {

                log_info("Flushing cache for sector %u entry\n", entry->sector);
                mutex_release(&tableLock);
                mutex_lock(&entry->lock);

                ata_write(entry->sector, SECTORS_PER_PAGE, entry->page);
                mm_pagination_clean(entry->page);

                mutex_release(&entry->lock);
                mutex_lock(&tableLock);
            }
        }
    }
    mutex_release(&tableLock);

    return 0;
}

int cache_evict(int minPages) {

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

    struct CacheReference* entry = cache_list.last;
    while (entry && evicted < minPages) {

        if ((!mm_pagination_is_dirty(entry->page) && entry->atime < now) || force) {
            evicted += release_chunk(entry->index);
        }

        entry = entry->prev;
    }

    return evicted;
}

int release_chunk(int tableIndex) {

    struct CacheReference* entry = table[tableIndex];
    kprintf("Contemplating to release chunk for sector %u\n", entry->sector);

    mutex_lock(&entry->lock);

    void* page = entry->page;
    entry->page = NULL;
    entry->evicting = 1;
    if (mm_pagination_is_dirty(page)) {
        ata_write(entry->sector, SECTORS_PER_PAGE, page);
    }

    if (entry->evicting) {
        log_info("Releasing chunk for sector %u\n", entry->sector);
        kprintf("Releasing chunk for sector %u\n", entry->sector);

        freePages(page, 1);
        entry->evicting = 0;

        mutex_lock(&tableLock);
        cache_list_remove(&cache_list, entry);
        table[tableIndex] = NULL;
        mutex_release(&tableLock);

        mutex_release(&entry->lock);

        if (entry->refs == 0) {
            kfree(entry);
        } else {
            entry->next = evicted;
            evicted = entry;

            if (entry->next) {
                entry->next->prev = entry;
            }
        }

        return 1;
    } else {

        entry->page = page;
        mutex_release(&entry->lock);

        return 0;
    }
}

struct CacheReference* cache_get(unsigned long long sector) {

    sector -= sector % SECTORS_PER_PAGE;
    struct CacheReference* ref = find_chunk(sector);
    if (ref == NULL) {
        return NULL;
    }

    ref->refs++;
    cache_repopulate(ref);
    return ref;
}

void cache_repopulate(struct CacheReference* ref) {

    if (ref->evicting) {
        // If we are lucky, we might just avoid eviction
        ref->evicting = 0;
    }

    mutex_lock(&ref->lock);

    if (!ref->page) {

        mutex_lock(&tableLock);
        int i;
        for (i = 0; i < TABLE_ENTRIES; i++) {
            if (!table[i]) {
                break;
            }
        }

        if (i == TABLE_ENTRIES) {

            if (cache_evict(1) < 1) {
                // Screwed
                mutex_release(&ref->lock);
                mutex_release(&tableLock);
                return;
            }

            for (i = 0; i < TABLE_ENTRIES; i++) {
                if (!table[i]) {
                    break;
                }
            }
        }

        ref->page = reserve_page();
        if (ref->page != NULL) {

            table[i] = ref;
            if (ref->next) {
                ref->next->prev = ref->prev;
            }

            if (ref->prev) {
                ref->prev->next = ref->next;
            } else {
                evicted = ref->next;
            }

            mutex_release(&tableLock);

            ata_read(ref->sector, SECTORS_PER_PAGE, ref->page);
        } else {
            mutex_release(&tableLock);
        }
    }

    mutex_release(&ref->lock);
}

void cache_release(struct CacheReference* ref) {
    ref->refs--;
}

void cache_access(struct CacheReference* ref) {
    ref->atime = _time(NULL);

    cache_list_remove(&cache_list, ref);
    cache_list_add(&cache_list, ref);
}

void* reserve_page(void) {

    void* page = allocPages(1);
    if (page == NULL && cache_evict(1) > 0) {
        page = allocPages(1);
    }

    return page;
}

void cache_freeze(struct CacheReference* ref) {

    if (ref->frozen) {
        return;
    }

    mutex_lock(&ref->lock);

    cache_list_remove(&cache_list, ref);

    ref->next = freezer;
    freezer = ref;

    ref->frozen = 1;

    mutex_release(&ref->lock);
}

