#ifndef __BITMAP__
#define __BITMAP__

#include "type.h"
#define BYTE(map, bit) (map[bit / sizeof(unsigned int)])

inline static void bitmap_set(unsigned int* map, int bit) {
    BYTE(map, bit) |= 0x1 << (bit % sizeof(unsigned int));
}

inline static void bitmap_clear(unsigned int* map, int bit) {
    BYTE(map, bit) &= ~(0x1 << (bit % sizeof(unsigned int)));
}

inline static int bitmap_test(unsigned int* map, int bit) {
    return (BYTE(map, bit) >> (bit % sizeof(unsigned int))) & 0x1;
}

inline static int bitmap_first_clear(unsigned int* map, size_t slots) {

    for (size_t i = 0; i < slots; i++) {

        if (map[i] != ~((unsigned int) 0)) {
            for (size_t j = 0; j < sizeof(unsigned int); j++) {

                if (!bitmap_test(map, i * sizeof(unsigned int) + j)) {
                    return i * sizeof(unsigned int) + j;
                }
            }
        }
    }

    return -1;
}
#undef BYTE


#endif
