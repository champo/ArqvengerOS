#include "drivers/keyboard.h"
#include "system/reboot.h"
#include "drivers/video.h"
#include "system/call/ioctl/keyboard.h"
#include "system/io.h"
#include "system/common.h"

#define KEYBOARD_IO_PORT 0x60

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

#define CAPS_LOCK 0x3A
#define SCROLL_LOCK 0x46
#define NUM_LOCK 0x45

static char normalCodeTable[] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, 0,
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' '
};
static char shiftCodeTable[] = {
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, 0,
        'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
};
static char altCodeTable[] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, 0,
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' '
};

static struct ModifierStatus {
    int ctrl;
    int alt;
    int shift;
    int caps;
    int scroll;
    int num;
} kbStatus = {0, 0, 0, 0, 0, 0};

static termios status = { 1, 1 };

static int escaped = 0, bufferEnd = 0;

static char inputBuffer[BUFFER_SIZE];

void echo(const char* str, size_t len);

void echo(const char* str, size_t len) {
    if (status.echo) {
        writeScreen(str, len);
    }
}

void readScanCode() {

    unsigned char scanCode = inB(KEYBOARD_IO_PORT);

    if (scanCode == ESCAPED_CODE) {
        // This is an escaped code, for now we bail
        escaped = 1;
        return;
    }

    int isBreak = scanCode >= MIN_BREAK_CODE;
    int makeCode = scanCode - isBreak * MIN_BREAK_CODE;

    char* codeTable;
    if (kbStatus.shift) {
        codeTable = shiftCodeTable;
    } else if (kbStatus.alt) {
        codeTable = altCodeTable;
    } else {
        codeTable = normalCodeTable;
    }

    if (escaped || makeCode > LAST_CODE_IN_TABLE || !codeTable[makeCode]) {
        // This can't be mapped to an ascii, so let's handle it on it's own
        // TODO: Take into account keys like arrows, and those
        switch (makeCode) {
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
            case DELETE_CODE:
                if (kbStatus.alt && kbStatus.ctrl && escaped) {
                    reboot();
                }
                break;
            case CAPS_LOCK:
                if (!isBreak) {
                    kbStatus.caps = !kbStatus.caps;
                    //TODO: Set the light
                }
                break;
            case NUM_LOCK:
                if (!isBreak) {
                    //TODO: Set the light
                    kbStatus.num = !kbStatus.num;
                }
                break;
            case SCROLL_LOCK:
                if (!isBreak) {
                    //TODO: Set the light
                    //TODO: Make this stop execution
                    kbStatus.scroll = !kbStatus.scroll;
                }
                break;
        }
    } else if (bufferEnd < BUFFER_SIZE && !isBreak) {
        // We know how to map this, yay!
        inputBuffer[bufferEnd++] = codeTable[makeCode];
        echo(&codeTable[makeCode], 1);
    }

    escaped = 0;
}

size_t readKeyboard(void* buffer, size_t count) {

    char* buf = (char*) buffer;
    int i = 0, c = (int) count;

    if (status.canon) {

        // Make sure until we read a whole line, and then only take what we need.
        for (i = 0; i < bufferEnd && inputBuffer[i] != '\n'; i++);
        if (i == bufferEnd) {

            // Oh noes! No full line was ever read!
            // Lets wait until we have one
            int interruptDisabled = !_isIF();

            enableInterrupts();
            while (bufferEnd == 0 || inputBuffer[bufferEnd - 1] != '\n') {
                halt();
            }

            if (interruptDisabled) {
                disableInterrupts();
            }

            i = bufferEnd;
        }

        if (c > i) {
            // If we were asked for more than the len of this line, we ignore that
            c = i;
        }
    } else {

        // Wait until the buffer is done
        int interruptDisabled = !_isIF();

        enableInterrupts();
        while (bufferEnd < c) {
            halt();
        }

        if (interruptDisabled) {
            disableInterrupts();
        }
    }

    // Copy the input to the buffer
    for (i = 0; i < c; i++) {
        buf[i] = inputBuffer[i];
    }

    // Make sure we move the input buffer to remove what was already read
    for (i = 0; c < bufferEnd; i++, count++) {
        inputBuffer[i] = inputBuffer[count];
    }
    bufferEnd -= c;

    return c;
}

int ioctlKeyboard(int cmd, void* argp) {

    termios* param;
    int res = 0;

    switch (cmd) {
        case TCGETS:
            param = (termios*) argp;
            *param = status;
            break;
        case TCSETS:
            param = (termios*) argp;
            status = *param;
            break;
        default:
            res = -1;
            break;
    }

    return res;
}

