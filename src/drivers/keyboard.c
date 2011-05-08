#include "drivers/keyboard.h"
#include "common.h"

#define KEYBOARD_READ_PORT 0x60
#define LAST_CODE_IN_TABLE 0x39
#define MIN_BREAK_CODE 0x80
#define ESCAPE_OFFSET 0xFF

#define ESCAPED_CODE 0xE0
#define BACKSPACE_CODE 0x0E
#define CTRL_CODE 0x1D
#define LSHIFT_CODE 0x2A
#define RSHIFT_CODE 0x36
#define ALT_CODE 0x38
#define ENTER_CODE 0x1C
#define DELETE_CODE 0x53

#define BUFFER_SIZE 4000

char makeCodeToAscii[] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, 0,
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' '
};

struct ModifierStatus {
    int ctrl;
    int alt;
    int shift;
} kbStatus;

int escaped = 0, bufferEnd = 0;

char inputBuffer[BUFFER_SIZE];

void int09() {

    unsigned char scanCode = inB(KEYBOARD_READ_PORT);

    if (scanCode == ESCAPED_CODE) {
        // This is an escaped code, for now we bail
        escaped = 1;
        return;
    }

    int isBreak = scanCode >= MIN_BREAK_CODE;
    int makeCode = scanCode - isBreak * MIN_BREAK_CODE;

    if (escaped || makeCode > LAST_CODE_IN_TABLE || !makeCodeToAscii[makeCode]) {
        // This can't be mapped to an ascii, so let's handle it on it's own
        switch (makeCode) {
            case BACKSPACE_CODE:
                if (!isBreak) {
                    bufferEnd--;
                    if (bufferEnd < 0) {
                        bufferEnd = 0;
                    }
                }
                break;
            case CTRL_CODE:
                kbStatus.ctrl = !isBreak;
                break;
            case LSHIFT_CODE:
            case RSHIFT_CODE:
                kbStatus.shift = !isBreak;
                break;
            case ALT_CODE:
                kbStatus.alt = !isBreak;
                break;
            case ENTER_CODE:
                if (bufferEnd < BUFFER_SIZE && !isBreak) {
                    inputBuffer[bufferEnd++] = '\n';
                }
                break;
            case DELETE_CODE:
                if (kbStatus.alt && kbStatus.ctrl && !escaped) {
                    //TODO: Reboot here
                }
                break;
        }
    } else if (bufferEnd < BUFFER_SIZE && !isBreak) {
        // We know how to map this, yay!
        //TODO: How bout we take the modifier keys into account? Crazy ain't it
        inputBuffer[bufferEnd++] = makeCodeToAscii[makeCode];
    }

    escaped = 0;
}

size_t read(int fd, void* buffer, size_t count) {
    // Well, we'll ignore the fd for now.
    char* buf = (char*) buffer;
    int i = 0;

    // Wait until the buffer is done
    while (bufferEnd < count) {
        halt();
    }

    // Copy the input to the buffer
    for (i = 0; i < count; i++) {
        buf[i] = inputBuffer[i];
    }

    // Make sure we move the input buffer to remove what was already read
    for (i = 0; count < bufferEnd; i++, count++) {
        inputBuffer[i] = inputBuffer[count];
    }
    bufferEnd -= count;

    return count;
}
