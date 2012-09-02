#ifndef __SYSTEM_MM_ALLOCATOR__
#define __SYSTEM_MM_ALLOCATOR__

#include "multiboot.h"
#include "type.h"
#include "system/process/process.h"
#include "system/mm/page.h"
#include "system/alloc.h"

void initMemoryMap(struct multiboot_info* info);

void* allocPages(size_t pages);

void freePages(void* page, size_t pages);

void mm_set_kernel_context(void);

void mm_set_process_context(void);

void* mm_create_context(void* heap, size_t size);

struct Pages* reserve_pages(struct Process* owner, int pages);

void free_pages(struct Pages* pages);

#endif
