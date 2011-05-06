#include "../include/keyboard.h"
#include "../include/video.h"

#define KEYBOARD_READ_PORT 0x60

void int09() {
    unsigned char scanCode = inB(KEYBOARD_READ_PORT);

    char out[3] = {0, ' ', ' '};
    int i = 2;
    while (scanCode) {
        out[i--] = '0' + (scanCode % 10);
        scanCode /= 10;
    }

    write(out, 3);
}
