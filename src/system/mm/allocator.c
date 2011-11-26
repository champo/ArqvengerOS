#include "system/mm/allocator.h"
#include "library/stdio.h"
#include "system/common.h"
#include "system/panic.h"
#include "system/scheduler.h"
#include "system/malloc/malloc.h"
#include "system/cache/cache.h"

struct MemoryMapEntry {
    size_t size;
    unsigned int base_addr_low;
    unsigned int base_addr_high;
    unsigned int length_low;
    unsigned int length_high;
    unsigned int type;
};

#define MAPPABLE_PAGES (1024 * 1024u)

#define UNUSABLE_PAGES 1024u
#define MEMORY_START (UNUSABLE_PAGES * PAGE_SIZE)

#define USABLE_PAGES (MAPPABLE_PAGES - UNUSABLE_PAGES)


#define PAGES_PER_ENTRY ((unsigned int) sizeof(int) * 8)
#define ENTRIES_IN_MAP ((USABLE_PAGES / PAGES_PER_ENTRY) + 1)

static int* pageMap;

static void* mallocContext;

inline static void setPage(int page);

inline static void unsetPage(int page);

inline static int isPageSet(int page);

static void initPages(struct multiboot_info* info);

static void reservePageMap(struct multiboot_info* info);

static void* allocator(size_t size, size_t* allocatedSize);

static void* find_pages(size_t pages);

void initMemoryMap(struct multiboot_info* info) {

    if (info->flags & (0x1 << 6)) {

        reservePageMap(info);
        initPages(info);

        init_malloc(allocPages(1), PAGE_SIZE);
        set_external_alloc(allocator);

        mallocContext = get_malloc_context();

    } else {
        panic();
    }
}

void initPages(struct multiboot_info* info) {

    size_t totalMem = 0;
    size_t usablePages = 0;

    struct MemoryMapEntry* entry = (struct MemoryMapEntry*) info->mmap_addr;
    while ((size_t) entry < info->mmap_addr + info->mmap_length) {

        size_t end = entry->base_addr_low + entry->length_low;
        if (!entry->base_addr_high && end) {

            if (entry->type == 1 && end >= MEMORY_START) {

                size_t start = entry->base_addr_low;
                if (start < MEMORY_START) {
                    start = MEMORY_START;
                }

                size_t firstPage = start / PAGE_SIZE;

                size_t startOffset = start % PAGE_SIZE;
                if (startOffset) {
                    firstPage++;
                }

                size_t alignedLength = entry->length_low - startOffset;
                size_t pages = alignedLength / PAGE_SIZE;

                for (size_t page = firstPage; page < firstPage + pages; page++) {
                    if (page < MAPPABLE_PAGES) {
                        unsetPage(page);
                    }
                }

                totalMem += pages * PAGE_SIZE;
                usablePages += pages;
            }
        }

        entry = (struct MemoryMapEntry*) ((char*) entry + entry->size + sizeof(unsigned int));
    }
}

void reservePageMap(struct multiboot_info* info) {

    size_t mapSize = sizeof(int) * ENTRIES_IN_MAP;

    struct MemoryMapEntry* entry = (struct MemoryMapEntry*) info->mmap_addr;
    while ((size_t) entry < info->mmap_addr + info->mmap_length) {

        size_t end = entry->base_addr_low + entry->length_low;
        if (!entry->base_addr_high && end) {

            if (entry->type == 1 && end >= 3 * 1024 * 1024 && entry->base_addr_low < MEMORY_START) {

                if (entry->base_addr_low < end - mapSize) {
                    if (end > MEMORY_START) {
                        pageMap = (int*) (3 * 1024 * 1024u);
                    } else {
                        pageMap = (int*) (end - mapSize);
                    }
                    for (size_t i = 0; i < ENTRIES_IN_MAP; i++) {
                        pageMap[i] = 0;
                    }

                    return;
                }
            }
        }

        entry = (struct MemoryMapEntry*) ((char*) entry + entry->size + sizeof(unsigned int));
    }

    panic();
}

void setPage(int page) {
    page -= UNUSABLE_PAGES;
    pageMap[page / PAGES_PER_ENTRY] &= -1 ^ (0x1 << (page % PAGES_PER_ENTRY));
}

void unsetPage(int page) {
    page -= UNUSABLE_PAGES;
    pageMap[page / PAGES_PER_ENTRY] |= 0x1 << (page % PAGES_PER_ENTRY);
}

int isPageSet(int page) {
    page -= UNUSABLE_PAGES;
    return !(pageMap[page / PAGES_PER_ENTRY] & (0x1 << (page % PAGES_PER_ENTRY)));
}

void* kalloc(size_t size) {
    return malloc(size);
}

void kfree(void* data) {
    free(data);
}

void* allocPages(size_t pages) {

    void* start = find_pages(pages);
    if (start == NULL) {
        log_debug("Not enough free memory, trying to evict pages from cache...\n");
        if (cache_evict(pages) >= pages) {
            return find_pages(pages);
        }
        log_error("Still not enough free memory, things are gonna go boom\n");

        return NULL;
    }

    return start;
}

void* find_pages(size_t pages) {

    if (pages == 0) {
        return NULL;
    }

    size_t start;
    for (size_t i = 0; i < ENTRIES_IN_MAP; i++) {
        if (pageMap[i]) {

            start = i * PAGES_PER_ENTRY + UNUSABLE_PAGES;

            size_t consecutive = 0;
            size_t offset;
            for (offset = 0;; offset++) {
                if (isPageSet(start + offset)) {
                    consecutive = 0;
                } else {
                    consecutive++;
                    if (consecutive == pages) {
                        break;
                    }
                }

                if (((offset + 1) % PAGES_PER_ENTRY) == 0) {
                    if (consecutive != 0 && i + 1 < ENTRIES_IN_MAP && pageMap[i+1]) {
                        i++;
                    } else {
                        break;
                    }
                }
            }

            if (consecutive == pages) {
                start += offset - pages + 1;
                break;
            }
        }

        start = 0;
    }

    if (start == 0) {
        return NULL;
    }

    for (size_t i = 0; i < pages; i++) {
        setPage(start + i);
    }

    return (void*)(start * PAGE_SIZE);
}

void freePages(void* page, size_t pages) {

    size_t start = ((unsigned int) page) / PAGE_SIZE;
    for (size_t i = 0; i < pages; i++) {
        unsetPage(start + i);
    }
}

void* allocator(size_t size, size_t* allocatedSize) {

    size_t pages = (size / PAGE_SIZE) + 1;
    void* memory = allocPages(pages);

    if (memory) {
        *allocatedSize = PAGE_SIZE * pages;
    } else {
        log_error("Failed to allocate memory for context %u\n", get_malloc_context());
        *allocatedSize = 0;
    }

    return memory;
}

void mm_set_kernel_context(void) {
    set_malloc_context(mallocContext);
}

void mm_set_process_context(void) {
    struct Process* p = scheduler_current();
    if (p != NULL && p->mm.mallocContext) {
        set_malloc_context(p->mm.mallocContext);
    } else {
        mm_set_kernel_context();
    }
}

void* mm_create_context(void* heap, size_t size) {
    init_malloc(heap, size);
    void* ctx = get_malloc_context();
    mm_set_kernel_context();
    return ctx;
}

struct Pages* reserve_pages(struct Process* owner, int pages) {

    void* start = allocPages(pages);
    if (start == NULL) {
        log_error("Failed to allocate %d pages for %d\n", pages, owner->pid);
        return NULL;
    }

    struct Pages* result = kalloc(sizeof(struct Pages));
    result->next = NULL;
    result->count = pages;
    result->start = start;

    result->next = owner->mm.reservedPages;
    owner->mm.reservedPages = result;

    return result;
}

void free_pages(struct Pages* pages) {

    struct Pages* current = pages;
    while (current) {
        freePages(current->start, current->count);

        struct Pages* next = current->next;
        kfree(current);
        current = next;
    }
}

