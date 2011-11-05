#ifndef __SYSTEM_MM_PAGE__
#define __SYSTEM_MM_PAGE__

#define PAGE_SIZE (4 * 1024u)

struct Pages {
    void* start;
    int count;
    struct Pages* next;
};

#endif
