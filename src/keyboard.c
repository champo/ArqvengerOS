#include "../include/keyboard.h"

#define KEYBOARD_READ_PORT 0x61

void int09() {
    unsigned char scanCode = inB(KEYBOARD_READ_PORT);

    char* video = (char*) 0xb8000;
    char out[3] = {0, ' ', ' '};
    int i = 2;
    while (scanCode) {
        out[i--] = '0' + scanCode % 10;
        scanCode /= 10;
    }
}
