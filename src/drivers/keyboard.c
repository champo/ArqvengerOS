#include "drivers/keyboard.h"
#include "system/reboot.h"
#include "drivers/tty/tty.h"
#include "system/call/ioctl/keyboard.h"
#include "system/io.h"
#include "system/common.h"
#include "system/scheduler.h"

#define KEYBOARD_IO_PORT 0x60
#define KEYBOARD_CTRL_PORT 0x64

#define LED_CODE 0xED

#define BUFFER_SIZE 4000

static unsigned char buffer[BUFFER_SIZE];

static int bufferPos = 0;

static int bufferStart = 0;

/**
 * Set the keyboard leds using the global state.
 */
void keyboard_leds(int caps, int num, int scroll) {

    byte leds = 0;

    if (caps) {
        leds |= 4;
    }

    if (num) {
        leds |= 2;
    }

    if (scroll) {
        leds |= 1;
    }

    // The keyboard buffer needs to be empty before setting these values
    // We assume that we won't lose any values, since we have just read something.
    while (inB(KEYBOARD_CTRL_PORT) & 0x2);
    outB(KEYBOARD_IO_PORT, LED_CODE);

    while (inB(KEYBOARD_CTRL_PORT) & 0x2);
    outB(KEYBOARD_IO_PORT, leds);
}

/**
 * Read & processs a scan code from the keyboard buffer.
 */
void keyboard_read(void) {

    unsigned char scanCode = inB(KEYBOARD_IO_PORT);
    buffer[bufferPos++] = scanCode;
    if (bufferPos == BUFFER_SIZE) {
        bufferPos = 0;
    }
}

unsigned char keyboard_get_code(void) {

    unsigned char ret = 0;
    if (bufferPos != bufferStart) {
        ret = buffer[bufferStart++];
        if (bufferStart == BUFFER_SIZE) {
            bufferStart = 0;
        }
    }

    return ret;
}

