#ifndef __SYSTEM_MM__
#define __SYSTEM_MM__

#include "multiboot.h"
#include "type.h"

#define PAGE_SIZE (4 * 1024u)

void initMemoryMap(struct multiboot_info* info);

void* allocPage(void);

/**
 * Alloc size bytes padded to PAGE_SIZE
 */
void* kalloc(size_t size);

void* allocPages(size_t pages);

void freePages(void* page, size_t pages);

#endif
