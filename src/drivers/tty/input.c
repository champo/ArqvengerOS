#include "drivers/tty/tty.h"
#include "drivers/keyboard.h"
#include "system/reboot.h"
#include "system/call/ioctl/keyboard.h"
#include "system/common.h"
#include "system/scheduler.h"

#define LAST_CODE_IN_TABLE 0x39
#define MIN_BREAK_CODE 0x80
#define ESCAPE_CHAR 0x1B

#define ESCAPED_CODE 0xE0
#define BACKSPACE_CODE 0x0E
#define CTRL_CODE 0x1D
#define LSHIFT_CODE 0x2A
#define RSHIFT_CODE 0x36
#define ALT_CODE 0x38
#define ENTER_CODE 0x1C
#define DELETE_CODE 0x53

#define UP_ARROW_CODE 0x48
#define LEFT_ARROW_CODE 0x4B
#define DOWN_ARROW_CODE 0x50
#define RIGHT_ARROW_CODE 0x4D

#define HOME_CODE 0x47
#define END_CODE 0x4F

#define BUFFER_SIZE 4000

#define CAPS_LOCK 0x3A
#define SCROLL_LOCK 0x46
#define NUM_LOCK 0x45

#define F1_CODE 0x3B
#define F2_CODE 0x3C
#define F3_CODE 0x3D
#define F4_CODE 0x3E

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

static void set_leds(void);

/**
 * Add a string of input with a fixed lenght to the keyboard buffer.
 *
 * This transforms raw input as needed, and echos to screen if needed.
 *
 * @param str A string with at least len chars
 * @param len The number of chars to add from str
 */
void addInput(const char* str, size_t len) {

    size_t i;
    for (i = 0; i < len && bufferEnd < BUFFER_SIZE; i++) {

        if (status.canon && str[i] == '\b') {
            if (bufferEnd > 0) {
                bufferEnd--;
            }
        } else {
            inputBuffer[bufferEnd++] = str[i];
        }
    }

    if (status.echo) {

        if (str[0] == ESCAPE_CHAR) {
            // A escape char is not printable, so we transform into something that is.
            writeScreen("^[", 2);
            if (i > 1) {
                writeScreen(str + 1, i - 1);
            }
        } else {
            writeScreen(str, i);
        }
    }
}

/**
 * Read & processs a scan code from the keyboard buffer.
 */
void process_scancode(void) {

    unsigned char scanCode = keyboard_get_code();

    if (scanCode == ESCAPED_CODE) {
        // This is an escaped code, for now we bail
        escaped = 1;
        return;
    }

    int isBreak = scanCode >= MIN_BREAK_CODE;
    int makeCode = scanCode - isBreak * MIN_BREAK_CODE;

    char* codeTable;
    if (kbStatus.shift || kbStatus.caps) {
        codeTable = shiftCodeTable;
    } else if (kbStatus.alt) {
        codeTable = altCodeTable;
    } else {
        codeTable = normalCodeTable;
    }

    if (escaped || makeCode > LAST_CODE_IN_TABLE || !codeTable[makeCode]) {
        // These can't be mapped to an ascii, so let's handle it on it's own
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
                    set_leds();
                }
                break;
            case NUM_LOCK:
                if (!isBreak) {
                    kbStatus.num = !kbStatus.num;
                    set_leds();
                }
                break;
            case SCROLL_LOCK:
                if (!isBreak) {
                    //TODO: Make this stop execution
                    kbStatus.scroll = !kbStatus.scroll;
                    set_leds();
                }
                break;
            case UP_ARROW_CODE:
                if (!isBreak && (escaped || !kbStatus.num)) {
                    addInput("\033[A", 3);
                }
                break;
            case DOWN_ARROW_CODE:
                if (!isBreak && (escaped || !kbStatus.num)) {
                    addInput("\033[B", 3);
                }
                break;
            case RIGHT_ARROW_CODE:
                if (!isBreak && (escaped || !kbStatus.num)) {
                    addInput("\033[C", 3);
                }
                break;
            case LEFT_ARROW_CODE:
                if (!isBreak && (escaped || !kbStatus.num)) {
                    addInput("\033[D", 3);
                }
                break;
            case HOME_CODE:
                if (!isBreak && (escaped || !kbStatus.num)) {
                    addInput("\033[H", 3);
                }
                break;
            case END_CODE:
                if (!isBreak && (escaped || !kbStatus.num)) {
                    addInput("\033[F", 3);
                }
                break;
            default:
                if (F1_CODE <= makeCode && makeCode <= F4_CODE) {
                    char mod = makeCode - F1_CODE + 'A';
                    addInput("\033[[", 3);
                    addInput(&mod, 1);
                }
                break;
        }
    } else if (bufferEnd < BUFFER_SIZE && !isBreak) {
        // We know how to map this, yay!
        addInput(&codeTable[makeCode], 1);
    }

    escaped = 0;
}

/**
 * Read into buffer from the keyboard buffer.
 *
 * The behaviour of this function is determined by the canon flag in termios.
 * If set, this will behave as if input was line buffered, and will return
 * as much as one line. Otherwise, it will behave as the definition implies.
 *
 * @param buffer a place to write the output to.
 * @param count the number of bytes to read.
 *
 * @return the number of bytes read.
 */
size_t readKeyboard(void* buffer, size_t count) {

    char* buf = (char*) buffer;
    int i, c = (int) count;

    if (status.canon) {

        // Make sure until we read a whole line, and then only take what we need.
        for (i = 0; i < bufferEnd && inputBuffer[i] != '\n'; i++);

        while (i == bufferEnd) {
            scheduler_do();
            for (i = 0; i < bufferEnd && inputBuffer[i] != '\n'; i++);
        }

        i++;

        if (c > i) {
            // If we were asked for more than the len of this line, we ignore that
            c = i;
        }
    } else {

        while (bufferEnd < c) {
            scheduler_do();
        }
    }

    // Copy the input to the buffer
    for (i = 0; i < c; i++) {
        buf[i] = inputBuffer[i];
    }

    // Make sure we move the input buffer to remove what was already read
    for (i = 0; c < bufferEnd; i++, c++) {
        inputBuffer[i] = inputBuffer[c];
    }
    bufferEnd = i;

    return c;
}

/**
 * Interface into driver-specific details.
 *
 * @param cmd The command to execute.
 * @param argp Extra parameters as needed, or not, by the command to execute.
 *
 * @return a status code, 0 on success, -1 on error.
 */
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

void initKeyboard(void) {
    set_leds();
}

void set_leds(void) {
    keyboard_leds(kbStatus.caps, kbStatus.num, kbStatus.scroll);
}

