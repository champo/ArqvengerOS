#include "../include/keyboard.h"
#include "../include/video.h"

#define KEYBOARD_READ_PORT 0x60

void int09() {
    unsigned char scanCode = inB(KEYBOARD_READ_PORT);
    // Do something with this
}
