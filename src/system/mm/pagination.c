#include "system/mm/pagination.h"
#include "system/mm/allocator.h"

static struct PageDirectory* identityDirectory;

static void setup_directory_entry(struct PageDirectoryEntry* entry, int present, unsigned int address);

static void setup_table_entry(struct PageTableEntry* entry, int present, unsigned int address);

void mm_pagination_init(void) {

    identityDirectory = allocPages(1);

    for (size_t i = 0; i < 1024; i++) {

        struct PageTable* table = allocPages(1);

        setup_directory_entry(&identityDirectory->entries[i], 1, (unsigned int) table);
        for (size_t j = 0; j < 1024; j++) {
            setup_table_entry(&table->entries[j], 1, (1024 * i + j) * PAGE_SIZE);
        }
    }

    __asm__ __volatile__("mov %%eax, %%cr3"::"A"(identityDirectory):);

    unsigned int cr0;
    __asm__ __volatile__("mov %%cr0, %%eax":"=A"(cr0)::);
    cr0 |= 0x80000000;
    __asm__ __volatile__("mov %%eax, %%cr0"::"A"(cr0):);

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

void setup_table_entry(struct PageTableEntry* entry, int present, unsigned int address) {
    entry->present = present;
    entry->pageAddress = address >> 12;
    entry->rw = 1;
    entry->user = 1;
    entry->writeThrough = 0;
    entry->cacheDisable = 0;
    entry->accessed = 0;
    entry->dirty = 0;
    entry->_pat = 0;
    entry->_global = 0;
    entry->_ignore = 0;
}

