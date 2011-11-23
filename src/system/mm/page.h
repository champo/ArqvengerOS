#ifndef __SYSTEM_MM_PAGE__
#define __SYSTEM_MM_PAGE__

#define PAGE_SIZE (4 * 1024u)
#define MAX_PAGES_IN_STACK  1024


struct Pages {
    void* start;
    int count;
    struct Pages* next;
};

struct PageDirectoryEntry {
    unsigned int present:1;
    unsigned int rw:1;
    unsigned int user:1;
    unsigned int writeThrough:1;
    unsigned int cacheDisable:1;
    unsigned int accessed:1;
    unsigned int dirty:1;
    unsigned int _pageSize:1;
    unsigned int _ignore:4;
    unsigned int tableAddress:20;
} __attribute__((packed));

struct PageTableEntry {
    unsigned int present:1;
    unsigned int rw:1;
    unsigned int user:1;
    unsigned int writeThrough:1;
    unsigned int cacheDisable:1;
    unsigned int accessed:1;
    unsigned int dirty:1;
    unsigned int _pat:1;
    unsigned int _global:1;
    unsigned int _ignore:3;
    unsigned int pageAddress:20;
} __attribute__((packed));

struct PageDirectory {
    struct PageDirectoryEntry entries[1024];
};

struct PageTable {
    struct PageTableEntry entries[1024];
};

#endif
