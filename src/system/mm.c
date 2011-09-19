#include "system/mm.h"
#include "library/stdio.h"
#include "system/common.h"

struct MemoryMapEntry {
    size_t size;
    unsigned int base_addr_low;
    unsigned int base_addr_high;
    unsigned int length_low;
    unsigned int length_high;
    unsigned int type;
};

#define PAGE_SIZE (4 * 1024u)
#define MAPPABLE_PAGES (1024 * 1024u)

#define SYSTEM_PAGES 1024u
#define MEMORY_START (1024 * PAGE_SIZE)

#define USABLE_PAGES (MAPPABLE_PAGES - SYSTEM_PAGES)


#define PAGES_PER_ENTRY ((unsigned int) sizeof(int) * 8)
#define ENTRIES_IN_MAP ((USABLE_PAGES / PAGES_PER_ENTRY) + 1)

int* pageMap;

inline static void setPage(int page);

inline static void unsetPage(int page);

inline static int isPageSet(int page);

static void initPages(struct multiboot_info* info);

static void reservePageMap(struct multiboot_info* info);

void initMemoryMap(struct multiboot_info* info) {

    if (info->flags & 0x1) {
        printf("Lower mem %d Higher mem %u\n", info->mem_lower, info->mem_upper);
    }

    if (info->flags & (0x1 << 6)) {
        reservePageMap(info);
        initPages(info);
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

    printf("Usable memory %u in %d pages\n", totalMem, usablePages);
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
                        pageMap[i] = 0xFFFFFFFF;
                    }

                    return;
                }
            }
        }

        entry = (struct MemoryMapEntry*) ((char*) entry + entry->size + sizeof(unsigned int));
    }

    printf("We don't have a place to put the page map. Dying...\n");

    disableInterrupts();
    halt();
}

void setPage(int page) {
    page -= SYSTEM_PAGES;
    pageMap[page / PAGES_PER_ENTRY] |= 0x1 << (page % PAGES_PER_ENTRY);
}

void unsetPage(int page) {
    page -= SYSTEM_PAGES;
    pageMap[page / PAGES_PER_ENTRY] &= -1 ^ (0x1 << (page % PAGES_PER_ENTRY));
}

int isPageSet(int page) {
    page -= SYSTEM_PAGES;
    return pageMap[page / PAGES_PER_ENTRY] & (0x1 << (page % PAGES_PER_ENTRY));
}

