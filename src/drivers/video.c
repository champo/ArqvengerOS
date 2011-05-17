#include "type.h"
#include "drivers/video.h"
#include "drivers/videoControl.h"
#include "system/io.h"

/* Video attribute. White letters on black background. */
#define WHITE_TXT 0x07

#define CONTROL_BUFFER_LEN 40
#define readControlBuffer(def) parseControlBuffer(&mod1, &mod2, def)
#define endControlSequence() status->escaped = status->csi = status->controlBufferPos = 0

#define isdigit(x) ((x) >= '0' && (x) <= '9')

#define BLINK_ATTR 0x80

#define COLOR_BRIGHT_BLUE 0x9
#define COLOR_BRIGHT_GREEN 0xA
#define COLOR_BRIGHT_CYAN 0xB
#define COLOR_BRIGHT_RED 0xC
#define COLOR_BRIGHT_MAGENTA 0xD
#define COLOR_BRIGHT_WHITE 0xF

#define NUM_SCREENS 4

static void handleControlSequence(char cur);

static void setAttribute(int bg, int fg, int blink);

static void updateCursor(void);

static void parseControlBuffer(int* a, int* b, int def);

static void clearScreen(int type);

static void eraseLine(int type);

static void setBlank(int start, int end);

static void setCharacter(char c);

static void scrollLine(int delta);

static void setForeground(int color);

static void setBackground(int color);

static void setBlink(int blink);

static void changeScreen(int screen);

static void flipBuffer(int lineOffset);

static char* videoMemory = (char*) 0xb8000;

typedef struct {
    int cursorPosition;
    int escaped;
    int csi;
    char attribute;
    char controlBuffer[CONTROL_BUFFER_LEN];
    size_t controlBufferPos;
    char videoBuffer[2 * LINE_WIDTH * (TOTAL_ROWS + 1)];
} ScreenStatus;

static ScreenStatus screens[NUM_SCREENS];
static ScreenStatus* status;

void initScreen(void) {

    int i;
    for (i = NUM_SCREENS - 1; i >= 0; i--) {
        status = &screens[i];

        status->cursorPosition = 0;
        status->escaped = 0;
        status->csi = 0;

        setAttribute(COLOR_BLACK, COLOR_WHITE, 0);

        status->controlBufferPos = 0;

        clearScreen(CLEAR_ALL);
    }
}

void flipBuffer(int lineOffset) {

    int i;
    for (i = 0; i < 2 * LINE_WIDTH * TOTAL_ROWS; i++) {
        videoMemory[i] = status->videoBuffer[i];
    }
}

void changeScreen(int screen) {

    status = &screens[screen % NUM_SCREENS];

    flipBuffer(0);
    updateCursor();
}

void setAttribute(int bg, int fg, int blink) {

    status->attribute = (fg & 0xF) | ((bg & 0xF) << 4);
    if (blink) {
        status->attribute |= BLINK_ATTR;
    }
}

void setForeground(int color) {
    setAttribute((status->attribute >> 4) & 0x7, color, status->attribute & BLINK_ATTR);
}

void setBackground(int color) {
    setAttribute(color, status->attribute & 0xF, status->attribute & BLINK_ATTR);
}

void setBlink(int blink) {
    setAttribute((status->attribute >> 4) & 0x7, status->attribute & 0xF, blink);
}

void setCharacter(char c) {
    videoMemory[2*status->cursorPosition] = c;
    videoMemory[2*status->cursorPosition + 1] = status->attribute;

    status->videoBuffer[2*status->cursorPosition] = c;
    status->videoBuffer[2*status->cursorPosition + 1] = status->attribute;

    status->cursorPosition++;
}

void scrollLine(int delta) {

    if (delta > TOTAL_ROWS) {
        delta = TOTAL_ROWS;
    } else if (delta < 1) {
        delta = 1;
    }

    int i, j;
    // Move the video buffer up one line
    for (j = 0, i = delta * 2 * LINE_WIDTH; i < 2 * LINE_WIDTH * TOTAL_ROWS; i++, j++) {
        status->videoBuffer[j] = status->videoBuffer[i];
    }

    // Clear the lines that scrolled up
    for (i = LINE_WIDTH * (TOTAL_ROWS - delta); i < LINE_WIDTH * TOTAL_ROWS; i++) {
        status->videoBuffer[2 * i] = ' ';
        status->videoBuffer[2 * i + 1] = status->attribute;
    }

    // Reflect the changes to screen
    flipBuffer(0);
}

void eraseLine(int type) {

    int start, end, line = status->cursorPosition / LINE_WIDTH;
    switch (type) {
        case ERASE_ALL:
            start = line * LINE_WIDTH;
            end = (line + 1) * LINE_WIDTH;
            break;
        case ERASE_LEFT:
            start = line * LINE_WIDTH;
            end = status->cursorPosition;
            break;
        case ERASE_RIGHT:
        default:
            start = status->cursorPosition + 1;
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
            end = LINE_WIDTH * (status->cursorPosition / LINE_WIDTH);
            break;
        case CLEAR_BELOW:
        default:
            start = (1 + status->cursorPosition / LINE_WIDTH) * LINE_WIDTH;
            end = TOTAL_ROWS * LINE_WIDTH;
            break;
    }

    setBlank(start, end);
}

void setBlank(int start, int end) {

    while (start < end) {
        videoMemory[2*start] = ' ';
        videoMemory[2*start + 1] = status->attribute;

        status->videoBuffer[2*start] = ' ';
        status->videoBuffer[2*start + 1] = status->attribute;
        start++;
    }
}

void parseControlBuffer(int* a, int* b, int def) {

    // Set the null at the end, and then parse it
    if (status->controlBufferPos) {

        size_t i = 0;
        int t = 0;

        while (i < status->controlBufferPos && status->controlBuffer[i] != ';') {
            t = t*10 + (status->controlBuffer[i] - '0');
            i++;
        }
        *a = t;

        if (i != status->controlBufferPos) {
            i++;

            t = 0;
            while (i < status->controlBufferPos) {
                t = t*10 + (status->controlBuffer[i] - '0');
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
        if (!status->escaped) {

            if (cur != ESCAPE_CODE) {

                int end;
                switch (cur) {
                    case '\n':
                        status->cursorPosition = LINE_WIDTH * (status->cursorPosition / LINE_WIDTH + 1);
                        break;
                    case '\r':
                        status->cursorPosition -= status->cursorPosition % LINE_WIDTH;
                        break;
                    case '\t':
                        if ((status->cursorPosition % LINE_WIDTH) + 4 >= LINE_WIDTH) {
                            end = (status->cursorPosition / LINE_WIDTH + 1) - 1;
                        } else {
                            end = status->cursorPosition + 4;
                        }

                        while (status->cursorPosition < end) {
                            setCharacter(' ');
                        }
                        break;
                    case '\v':
                        status->cursorPosition += LINE_WIDTH;
                        break;
                    case '\b':
                        if (status->cursorPosition > 0) {
                            status->cursorPosition -= 1;
                            setCharacter(' ');
                            status->cursorPosition -= 1;
                        }
                        break;
                    default:
                        setCharacter(cur);
                        break;
                }

                if (status->cursorPosition >= LINE_WIDTH * TOTAL_ROWS) {
                    scrollLine(1);
                    status->cursorPosition -= LINE_WIDTH;
                } else if (status->cursorPosition < 0) {
                    status->cursorPosition = 0;
                }
            } else {
                // We have an escape code! Let's have some fun :D
                status->escaped = 1;
            }
        } else {

            if (status->csi) {

                // We've already read ^[[ so now we go on to check what command we're given
                if (isdigit(cur)) {
                    // This is probably a modifier for a sequence, store!
                    status->controlBuffer[status->controlBufferPos++] = cur;
                } else {
                    handleControlSequence(cur);
                }
            } else if (cur == CSI) {

                // We have the Control Sequence Indicator, yay!
                status->csi = 1;
            } else {

                // This wasn't anything we know, so we end the escape sequence
                endControlSequence();
            }
        }
    }

    updateCursor();

    return length;
}

void handleControlSequence(char cur) {

    int mod1, mod2;
    switch (cur) {
        case 'Z':
            readControlBuffer(0);
            changeScreen(mod1);
            endControlSequence();
            break;
        case 'A':
            // Cursor up!
            readControlBuffer(1);
            if (status->cursorPosition < LINE_WIDTH * mod1) {
                status->cursorPosition = status->cursorPosition % LINE_WIDTH;
            } else {
                status->cursorPosition -= LINE_WIDTH * mod1;
            }

            endControlSequence();
            break;
        case 'B':
            // Cursor down!
            readControlBuffer(1);
            if (status->cursorPosition + mod1 * LINE_WIDTH < LINE_WIDTH * TOTAL_ROWS) {
                status->cursorPosition += mod1 * LINE_WIDTH;
            } else {
                status->cursorPosition = (TOTAL_ROWS - 1) * LINE_WIDTH + (status->cursorPosition % LINE_WIDTH);
            }

            endControlSequence();
            break;
        case 'C':
            // Cursor right!
            readControlBuffer(1);
            if ((status->cursorPosition % LINE_WIDTH) + mod1 >= LINE_WIDTH) {
                status->cursorPosition += LINE_WIDTH - (status->cursorPosition % LINE_WIDTH);
            } else {
                status->cursorPosition += mod1;
            }

            endControlSequence();
            break;
        case 'D':
            /* Cursor left! */
            readControlBuffer(1);
            if ((status->cursorPosition % LINE_WIDTH) < mod1) {
                status->cursorPosition -= status->cursorPosition % LINE_WIDTH;
            } else {
                status->cursorPosition -= mod1;
            }

            endControlSequence();
            break;
        case 'E':
            // Next line
            readControlBuffer(1);
            if (status->cursorPosition >= (LINE_WIDTH * (TOTAL_ROWS - 1))) {
                status->cursorPosition -= status->cursorPosition % LINE_WIDTH;
            } else if (status->cursorPosition / LINE_WIDTH + mod1 >= TOTAL_ROWS) {
                status->cursorPosition = (TOTAL_ROWS - 1) * LINE_WIDTH;
            } else {
                status->cursorPosition += LINE_WIDTH * mod1 - (status->cursorPosition % LINE_WIDTH);
            }

            endControlSequence();
            break;
        case 'F':
            // Previous line
            readControlBuffer(1);
            status->cursorPosition -= LINE_WIDTH * mod1 + (status->cursorPosition % LINE_WIDTH);
            if (status->cursorPosition < 0) {
                status->cursorPosition = 0;
            }

            endControlSequence();
            break;
        case 'G':
            // Jump to column
            readControlBuffer(1);
            if (mod1 < 1) {
                mod1 = 1;
            }
            status->cursorPosition += ((mod1 - 1) % LINE_WIDTH) - ((status->cursorPosition) % LINE_WIDTH);

            endControlSequence();
            break;
        case 'H':
            // Jump cursor
            readControlBuffer(1);
            if (mod1 < 1) {
                mod1 = 1;
            }

            if (mod2 < 1) {
                mod2 = 1;
            }

            status->cursorPosition = LINE_WIDTH * ((mod1 - 1) % TOTAL_ROWS) + ((mod2 - 1) % LINE_WIDTH);

            endControlSequence();
            break;
        case ';':
            // This case is related to the previos, it's the coord separator for jump cursor
            status->controlBuffer[status->controlBufferPos++] = ';';
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
            if (mod1 < 1) {
                mod1 = 1;
            }

            status->cursorPosition = ((mod1 - 1) % TOTAL_ROWS) + (status->cursorPosition % LINE_WIDTH);

            endControlSequence();
            break;
        case 'z':
            readControlBuffer(1);
            mod1 = status->cursorPosition;
            status->cursorPosition = (TOTAL_ROWS - 1) * LINE_WIDTH;
            mod2 = 0;
            while (status->controlBufferPos > mod2) setCharacter(status->controlBuffer[mod2++]);
            status->cursorPosition = mod1;
            endControlSequence();
            break;
        case 'm':
            // Change color!
            readControlBuffer(0);
            switch (mod1) {
                case 1:
                    // Bold
                    // Isolate the foreground color to check for boldness
                    mod2 = status->attribute & 0xF;
                    if (mod2 != COLOR_BLACK && mod2 != COLOR_BROWN && mod2 < COLOR_GRAY) {
                        setForeground(mod2 + 0x8);
                    }
                    break;
                case 22:
                    // Not bold
                    // Isolate the foreground color to check for boldness
                    mod2 = status->attribute & 0xF;
                    if (mod2 >= COLOR_BRIGHT_BLUE && mod2 != COLOR_YELLOW) {
                        setForeground(mod2 - 0x8);
                    }
                    break;
                case 37:
                    // White bg
                    setBackground(COLOR_WHITE);
                    break;
                case 36:
                    // Cyan bg
                    setBackground(COLOR_CYAN);
                    break;
                case 35:
                    // Magenta bg
                    setBackground(COLOR_MAGENTA);
                    break;
                case 34:
                    // Blue bg
                    setBackground(COLOR_BLUE);
                    break;
                case 32:
                    // Green bg
                    setBackground(COLOR_GREEN);
                    break;
                case 31:
                    // Red bg
                    setBackground(COLOR_RED);
                    break;
                case 39:
                case 30:
                    // Black bg
                    setBackground(COLOR_BLACK);
                    break;
                case 49:
                case 47:
                    // White fg
                    setForeground(COLOR_WHITE);
                    break;
                case 46:
                    // Cyan fg
                    setForeground(COLOR_CYAN);
                    break;
                case 45:
                    // Magenta fg
                    setForeground(COLOR_MAGENTA);
                    break;
                case 44:
                    // Blue fg
                    setForeground(COLOR_BLUE);
                    break;
                case 43:
                    // Yellow fg
                    setForeground(COLOR_YELLOW);
                    break;
                case 42:
                    // Green fg
                    setForeground(COLOR_GREEN);
                    break;
                case 41:
                    // Red fg
                    setForeground(COLOR_RED);
                    break;
                case 40:
                    // Black fg
                    setForeground(COLOR_BLACK);
                    break;
                case 25:
                    // Time of the Angels
                    // If you blink, you die
                    setBlink(0);
                    break;
                case 5:
                    // Blink!
                    setBlink(1);
                    break;
                case 0:
                    // Reset
                    setBlink(0);
                    setForeground(COLOR_BLACK);
                    setBackground(COLOR_WHITE);
                    break;
            }

            endControlSequence();
            break;
        default:
            endControlSequence();
            break;
    }
}

void updateCursor(void) {

    // cursor LOW port to vga INDEX register
    outB(0x3D4, 0x0F);
    outB(0x3D5, (unsigned char)(status->cursorPosition&0xFF));

    // cursor HIGH port to vga INDEX register
    outB(0x3D4, 0x0E);
    outB(0x3D5, (unsigned char )((status->cursorPosition>>8)&0xFF));
}

