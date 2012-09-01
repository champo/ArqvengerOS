#ifndef __SYSTEM_MM_PAGING__
#define __SYSTEM_MM_PAGING__

#include "system/process/process.h"
#include "system/mm/page.h"

void mm_pagination_init(void);

void mm_pagination_set_kernel_directory(void);

void mm_pagination_set_process_directory(void);

void mm_pagination_map(struct Process* owner, unsigned int start, unsigned int to, int pages, int user, int rw);

void mm_pagination_clear_directory(struct PageDirectory* directory);

void* mm_translate_address(struct Process* owner, unsigned int address);

int mm_pagination_is_dirty(const void* page);

void mm_pagination_clean(const void* page);

#endif
