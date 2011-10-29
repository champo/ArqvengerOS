#ifndef __SYSTEM_MM__
#define __SYSTEM_MM__

#include "multiboot.h"
#include "type.h"

#define PAGE_SIZE (4 * 1024u)

void initMemoryMap(struct multiboot_info* info);

void* kalloc(size_t size);

void kfree(void* data);

void* allocPages(size_t pages);

void freePages(void* page, size_t pages);

void mm_set_kernel_context(void);

void mm_set_process_context(void);

void* mm_create_context(void* heap, size_t size);

#endif
