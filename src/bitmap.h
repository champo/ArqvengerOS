#ifndef __BITMAP__
#define __BITMAP__

#include "type.h"
#define ENTRIES_PER_SLOT (8 * sizeof(unsigned int))
#define BYTE(map, bit) (map[bit / ENTRIES_PER_SLOT])

inline static void bitmap_set(unsigned int* map, int bit) {
    BYTE(map, bit) |= 0x1 << (bit % ENTRIES_PER_SLOT);
}

inline static void bitmap_clear(unsigned int* map, int bit) {
    BYTE(map, bit) &= ~(0x1u << (bit % ENTRIES_PER_SLOT));
}

inline static int bitmap_test(unsigned int* map, int bit) {
    return (BYTE(map, bit) >> (bit % ENTRIES_PER_SLOT)) & 0x1;
}

inline static int bitmap_first_clear(unsigned int* map, size_t entries) {

    if (entries == -1) {
        return -1;
    }

    size_t slots = entries / ENTRIES_PER_SLOT;
    for (size_t i = 0; i < slots; i++) {

        if (map[i] != ~((unsigned int) 0)) {
            for (size_t j = 0; j < ENTRIES_PER_SLOT; j++) {

                size_t bit = i * ENTRIES_PER_SLOT + j;
                if (!bitmap_test(map, bit)) {
                    return bit;
                }
            }
        }
    }

    return -1;
}
#undef BYTE


#endif
