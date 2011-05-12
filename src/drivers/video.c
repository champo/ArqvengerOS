#include "type.h"
#include "drivers/video.h"
#include "system/io.h"

/* Video attribute. White letters on black background. */
#define WHITE_TXT 0x07

#define LINE_WIDTH 80
#define TOTAL_ROWS 24

#define ESCAPE_CODE 0x1B
#define CSI '['

#define CLEAR_ALL 2
#define CLEAR_ABOVE 1
#define CLEAR_BELOW 0

#define ERASE_ALL 2
#define ERASE_LEFT 1
#define ERASE_RIGHT 0

#define CONTROL_BUFFER_LEN 40
#define readControlBuffer(def) parseControlBuffer(&mod1, &mod2, def)
#define endControlSequence() escaped = csi = controlBufferPos = 0

#define isdigit(x) ((x) >= '0' && (x) <= '9')

void updateCursor();

void parseControlBuffer(int* a, int* b, int def);

void clearScreen(int type);

void eraseLine(int type);

void setBlank(int start, int end);

void setCharacter(char c);

static int cursorPosition = 0;
static int escaped = 0, csi = 0, attribute = WHITE_TXT;
static char controlBuffer[CONTROL_BUFFER_LEN];
static size_t controlBufferPos = 0;
static char* videoBuffer = (char*) 0xb8000;

void setCharacter(char c) {
    videoBuffer[2*cursorPosition] = c;
    videoBuffer[2*cursorPosition+1] = attribute;

    cursorPosition++;
}

void eraseLine(int type) {

    int start, end, line = cursorPosition / LINE_WIDTH;
    switch (type) {
        case ERASE_ALL:
            start = line * LINE_WIDTH;
            end = (line + 1) * LINE_WIDTH;
            break;
        case ERASE_LEFT:
            start = line * LINE_WIDTH;
            end = cursorPosition;
            break;
        case ERASE_RIGHT:
        default:
            start = cursorPosition + 1;
            end = (line + 1) * LINE_WIDTH;
            break;
    }

    setBlank(start, end);
}

void clearScreen(int type) {

    int start, end;
    switch (type) {
        case CLEAR_ALL:
            start = 0;
            end = TOTAL_ROWS * LINE_WIDTH;
            break;
        case CLEAR_ABOVE:
            start = 0;
            end = LINE_WIDTH * (cursorPosition / LINE_WIDTH);
            break;
        case CLEAR_BELOW:
        default:
            start = (1 + cursorPosition / LINE_WIDTH) * LINE_WIDTH;
            end = TOTAL_ROWS * LINE_WIDTH;
            break;
    }

    setBlank(start, end);
}

void setBlank(int start, int end) {

    while (start < end) {
        videoBuffer[2*start] = ' ';
        videoBuffer[2*start + 1] = attribute;
        start++;
    }
}

void parseControlBuffer(int* a, int* b, int def) {

    // Set the null at the end, and then parse it
    if (controlBufferPos) {

        size_t i = 0;
        int t = 0;

        while (i < controlBufferPos && controlBuffer[i] != ';') {
            t = t*10 + (controlBuffer[i] - '0');
            i++;
        }
        *a = t;

        if (i != controlBufferPos) {
            i++;
            while (i < controlBufferPos) {
                t = t*10 + (controlBuffer[i] - '0');
                i++;
            }

            *b = t;
        } else {
            *b = def;
        }
    } else {
        *a = def;
        *b = def;
    }
}

size_t writeScreen(const void* buf, size_t length) {

    const char* str = (const char*) buf;

    size_t aux = 0;
    char cur;

    for (aux = 0; aux < length; aux++) {
        cur = str[aux];
        if (!escaped) {

            if (cur != ESCAPE_CODE) {

                size_t end;
                switch (cur) {
                    case '\n':
                        cursorPosition = cursorPosition / LINE_WIDTH + 1;
                        break;
                    case '\r':
                        cursorPosition -= cursorPosition % LINE_WIDTH;
                        break;
                    case '\t':
                        if ((cursorPosition % LINE_WIDTH) + 4 >= LINE_WIDTH) {
                            end = (cursorPosition / LINE_WIDTH + 1) - 1;
                        } else {
                            end = cursorPosition + 4;
                        }

                        while (cursorPosition < end) {
                            setCharacter(' ');
                        }
                        break;
                    case '\v':
                        cursorPosition += LINE_WIDTH;
                        break;
                    case '\b':
                        if (cursorPosition > 0) {
                            cursorPosition -= 1;
                            setCharacter(' ');
                            cursorPosition -= 1;
                        }
                        break;
                    default:
                        setCharacter(cur);
                        break;
                }

                if (cursorPosition >= LINE_WIDTH * TOTAL_ROWS) {
                    cursorPosition = 0;
                }
            } else {
                // We have an escape code! Let's have some fun :D
                escaped = 1;
            }
        } else {

            if (csi) {

                // We've already read ^[[ so now we go on to check what command we're given
                if (isdigit(cur)) {
                    // This is probably a modifier for a sequence, store!
                    controlBuffer[controlBufferPos++] = cur;
                } else {
                    int mod1, mod2;
                    switch (cur) {
                        case 'A':
                            // Cursor up!
                            readControlBuffer(1);
                            if (cursorPosition < LINE_WIDTH * mod1) {
                                cursorPosition = cursorPosition % LINE_WIDTH;
                            } else {
                                cursorPosition -= LINE_WIDTH * mod1;
                            }

                            endControlSequence();
                            break;
                        case 'B':
                            // Cursor down!
                            readControlBuffer(1);
                            if (cursorPosition + mod1 * LINE_WIDTH < LINE_WIDTH * TOTAL_ROWS) {
                                cursorPosition += mod1 * LINE_WIDTH;
                            } else {
                                cursorPosition = (TOTAL_ROWS - 1) * LINE_WIDTH + (cursorPosition % LINE_WIDTH);
                            }

                            endControlSequence();
                            break;
                        case 'C':
                            // Cursor right!
                            readControlBuffer(1);
                            if ((cursorPosition % LINE_WIDTH) + mod1 >= LINE_WIDTH) {
                                cursorPosition = (cursorPosition / LINE_WIDTH) + 1;
                            } else {
                                cursorPosition += mod1;
                            }

                            endControlSequence();
                            break;
                        case 'D':
                            /* Cursor left! */
                            readControlBuffer(1);
                            if ((cursorPosition % LINE_WIDTH) < mod1) {
                                cursorPosition -= cursorPosition % LINE_WIDTH;
                            } else {
                                cursorPosition -= mod1;
                            }

                            endControlSequence();
                            break;
                        case 'E':
                            // Next line
                            readControlBuffer(1);
                            if (cursorPosition >= (LINE_WIDTH * (TOTAL_ROWS - 1))) {
                                cursorPosition -= cursorPosition % LINE_WIDTH;
                            } else if (cursorPosition / LINE_WIDTH + mod1 >= TOTAL_ROWS) {
                                cursorPosition = (TOTAL_ROWS - 1) * LINE_WIDTH;
                            } else {
                                cursorPosition += LINE_WIDTH * mod1 - (cursorPosition % LINE_WIDTH);
                            }

                            endControlSequence();
                            break;
                        case 'F':
                            // Previous line
                            readControlBuffer(1);
                            cursorPosition -= LINE_WIDTH * mod1 - (cursorPosition % LINE_WIDTH);
                            if (cursorPosition < 0) {
                                cursorPosition = 0;
                            }

                            endControlSequence();
                            break;
                        case 'G':
                            // Jump to column
                            readControlBuffer(1);
                            cursorPosition += ((mod1 - 1) % LINE_WIDTH) - ((cursorPosition) % LINE_WIDTH);

                            endControlSequence();
                            break;
                        case 'H':
                            // Jump cursor
                            readControlBuffer(1);
                            cursorPosition = LINE_WIDTH * ((mod1 - 1) % TOTAL_ROWS) + ((mod2 - 1) % LINE_WIDTH);

                            endControlSequence();
                            break;
                        case ';':
                            // This case is related to the previos, it's the coord separator for jump cursor
                            controlBuffer[controlBufferPos++] = ';';
                            break;
                        case 'J':
                            // Erase (above|below|all)
                            readControlBuffer(0);
                            clearScreen(mod1);

                            endControlSequence();
                            break;
                        case 'K':
                            // Erase to (left|right|all)
                            readControlBuffer(0);
                            eraseLine(mod1);

                            endControlSequence();
                            break;
                        case 'd':
                            // Jump to row
                            readControlBuffer(1);
                            cursorPosition = ((mod1 - 1) % TOTAL_ROWS) + (cursorPosition % LINE_WIDTH);

                            endControlSequence();
                            break;
                        case 'm':
                            // Change color!
                            endControlSequence();
                            break;
                        default:
                            endControlSequence();
                            break;
                    }
                }
            } else if (cur == CSI) {

                // We have the Control Sequence Indicator, yay!
                csi = 1;
            } else {

                // This wasn't anything we know, so we end the escape sequence
                endControlSequence();
            }
        }
    }

    updateCursor();

    return length;
}

void updateCursor() {
    // cursor LOW port to vga INDEX register
    outB(0x3D4, 0x0F);
    outB(0x3D5, (unsigned char)(cursorPosition&0xFF));

    // cursor HIGH port to vga INDEX register
    outB(0x3D4, 0x0E);
    outB(0x3D5, (unsigned char )((cursorPosition>>8)&0xFF));
}

