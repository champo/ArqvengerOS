#include "system/mm/pagination.h"
#include "system/mm/allocator.h"
#include "system/scheduler.h"
#include "debug.h"

static struct PageDirectory* identityDirectory;

static void set_directory(struct PageDirectory* directory);

static void setup_directory_entry(struct PageDirectoryEntry* entry, int present, unsigned int address);

static void setup_table_entry(struct PageTableEntry* entry, int present, unsigned int address, int user, int rw);

void mm_pagination_init(void) {

    identityDirectory = allocPages(1);

    for (size_t i = 0; i < 1024; i++) {

        struct PageTable* table = allocPages(1);

        setup_directory_entry(&identityDirectory->entries[i], 1, (unsigned int) table);
        for (size_t j = 0; j < 1024; j++) {
            setup_table_entry(&table->entries[j], 1, (1024 * i + j) * PAGE_SIZE, 0, 1);
        }
    }

    set_directory(identityDirectory);

    unsigned int cr0;
    __asm__ __volatile__("mov %%cr0, %%eax":"=A"(cr0)::);
    cr0 |= 0x80000000;
    __asm__ __volatile__("mov %%eax, %%cr0"::"A"(cr0):);
}

void set_directory(struct PageDirectory* directory) {
    __asm__ __volatile__("mov %%eax, %%cr3"::"A"(directory):);
}

void mm_pagination_set_kernel_directory(void) {
    set_directory(identityDirectory);
}

void mm_pagination_set_process_directory(void) {
    struct Process* p = scheduler_current();
    if (p && p->mm.directory) {
        set_directory(p->mm.directory);
    }
}

void setup_directory_entry(struct PageDirectoryEntry* entry, int present, unsigned int address) {
    entry->present = present;
    entry->tableAddress = address >> 12;
    entry->rw = 1;
    entry->user = 1;
    entry->writeThrough = 0;
    entry->cacheDisable = 0;
    entry->accessed = 0;
    entry->dirty = 0;
    entry->_pageSize = 0;
    entry->_ignore = 0;
}

void setup_table_entry(struct PageTableEntry* entry, int present, unsigned int address, int user, int rw) {
    entry->present = present;
    entry->pageAddress = address >> 12;
    entry->rw = rw;
    entry->user = user;
    entry->writeThrough = 0;
    entry->cacheDisable = 0;
    entry->accessed = 0;
    entry->dirty = 0;
    entry->_pat = 0;
    entry->_global = 0;
    entry->_ignore = 0;
}

void mm_pagination_map(struct Process* owner, unsigned int start, unsigned int to, int pages, int user, int rw) {

    struct PageDirectory* directory = owner->mm.directory;

    for (int i = 0; i < pages; i++) {

        unsigned int destination = to + i * PAGE_SIZE;

        int directoryIndex = (destination >> 22) & 0x3FF;
        int tableIndex = (destination >> 12) & 0x3FF;

        struct PageDirectoryEntry* dirEntry = &directory->entries[directoryIndex];
        if (!dirEntry->present) {
            struct Pages* mem = reserve_pages(owner, 1);
            assert(mem != NULL);

            struct PageTable* table = mem->start;
            setup_directory_entry(dirEntry, 1, (unsigned int) table);

            for (size_t j = 0; j < 1024; j++) {
                setup_table_entry(&table->entries[j], 0, 0, 0, 0);
            }
        }

        struct PageTable* table = (struct PageTable*) (dirEntry->tableAddress * PAGE_SIZE);

        if (table->entries[tableIndex].present) {
            log_error(
                "Trying to remap %u to %u in %s\n",
                table->entries[tableIndex].pageAddress,
                destination >> 12,
                __func__
            );
            continue;
        }

        setup_table_entry(&table->entries[tableIndex], 1, start + i * PAGE_SIZE, user, rw);
    }
}

void mm_pagination_clear_directory(struct PageDirectory* directory) {

    for (size_t i = 0; i < 1024; i++) {
        setup_directory_entry(&directory->entries[i], 0, 0);
    }
}

void* mm_translate_address(struct Process* owner, unsigned int address) {

    if (owner->mm.directory == NULL) {
        return (void*) address;
    }

    int directoryIndex = (address >> 22) & 0x3FF;
    int tableIndex = (address >> 12) & 0x3FF;

    struct PageDirectoryEntry* dirEntry = &owner->mm.directory->entries[directoryIndex];
    if (dirEntry->present) {
        struct PageTable* table = (struct PageTable*) (dirEntry->tableAddress * PAGE_SIZE);
        if (table->entries[tableIndex].present) {
            return (void*)((table->entries[tableIndex].pageAddress * PAGE_SIZE) + (address & 0xFFF));
        }
    }

    return NULL;
}

