#include "type.h"
#include "drivers/video.h"
#include "drivers/tty/tty.h"
#include "drivers/tty/status.h"
#include "drivers/videoControl.h"

/* Video attribute. White letters on black background. */
#define WHITE_TXT 0x07

#define readControlBuffer(def) parseControlBuffer(term, &mod1, &mod2, def)
#define endControlSequence() status->escaped = status->csi = status->controlBufferPos = 0

#define isdigit(x) ((x) >= '0' && (x) <= '9')

#define BLINK_ATTR 0x80

#define COLOR_BRIGHT_BLUE 0x9
#define COLOR_BRIGHT_GREEN 0xA
#define COLOR_BRIGHT_CYAN 0xB
#define COLOR_BRIGHT_RED 0xC
#define COLOR_BRIGHT_MAGENTA 0xD
#define COLOR_BRIGHT_WHITE 0xF

static void handleControlSequence(struct Terminal* term, char cur);

static void setAttribute(struct Terminal* term, int bg, int fg, int blink);

static void parseControlBuffer(struct Terminal* term, int* a, int* b, int def);

static void clearScreen(struct Terminal* term, int type);

static void eraseLine(struct Terminal* term, int type);

static void setBlank(struct Terminal* term, int start, int end);

static void setCharacter(struct Terminal* term, char c);

static void scrollLine(struct Terminal* term, int delta);

static void setForeground(struct Terminal* term, int color);

static void setBackground(struct Terminal* term, int color);

static void setBlink(struct Terminal* term, int blink);

static void changeColor(struct Terminal* term, int mod1);

static void change_character(struct Terminal* term, int pos, char value, char attribute);

static void flip_buffer(struct Terminal* term);

static void update_cursor(struct Terminal* term);

static size_t tty_write_to_screen(struct Terminal* term, const void* buf, size_t length);

static int is_visible(struct Terminal* term, int pos);

/**
 * Initialize the screen and all needed status for tty emulation.
 */
void tty_screen_init(void) {

    for (int i = NUM_TERMINALS - 1; i >= 0; i--) {
        struct ScreenStatus* status = &tty_terminal(i)->screen;

        status->cursorPosition = 0;
        status->escaped = 0;
        status->csi = 0;

        status->lastLine = BUFFER_ROWS - TOTAL_ROWS;
        status->visibleLine = BUFFER_ROWS - TOTAL_ROWS;
        status->writableBuffer = status->videoBuffer + 2 * LINE_WIDTH * status->visibleLine;

        setAttribute(tty_terminal(i), COLOR_BLACK, COLOR_WHITE, 0);

        status->controlBufferPos = 0;

        clearScreen(tty_terminal(i), CLEAR_ALL);
    }
}

void tty_screen_scroll(struct Terminal* term, int line) {

    term->screen.visibleLine += line;
    if (term->screen.visibleLine < term->screen.lastLine) {
        term->screen.visibleLine = term->screen.lastLine;
    } else if (term->screen.visibleLine > BUFFER_ROWS - TOTAL_ROWS) {
        term->screen.visibleLine = BUFFER_ROWS - TOTAL_ROWS;
    }

    flip_buffer(term);
    update_cursor(term);
}

int is_visible(struct Terminal* term, int pos) {

    int bufferLine = (pos / LINE_WIDTH) + (BUFFER_ROWS - TOTAL_ROWS);
    int visibleLine = term->screen.visibleLine;
    return bufferLine >= visibleLine && bufferLine < visibleLine + TOTAL_ROWS;
}

void change_character(struct Terminal* term, int pos, char value, char attribute) {

    if (term->active && is_visible(term, pos)) {
        video_set_char(pos, value, attribute);
    }
}

void flip_buffer(struct Terminal* term) {

    if (term->active) {
        video_flip_buffer(term->screen.videoBuffer + 2 * term->screen.visibleLine * LINE_WIDTH);
    }
}

void update_cursor(struct Terminal* term) {

    if (term->active) {
        if (is_visible(term, term->screen.cursorPosition)) {
            video_update_cursor(term->screen.cursorPosition);
        } else {
            video_update_cursor(TOTAL_ROWS * LINE_WIDTH + 1);
        }
    }
}

/**
 * Change the active screen. This refreshes the screen.
 *
 * @param screen the number of the screen to set.
 */
void tty_screen_change(void) {

    struct Terminal* term = tty_active();

    flip_buffer(term);
    update_cursor(term);
}

/**
 * Set the video attribute.
 *
 * @param bg The background color.
 * @param fg The foreground color.
 * @param blink Whether to blink or not.
 */
void setAttribute(struct Terminal* term, int bg, int fg, int blink) {

    /*
     * The attribute byte is split into 3. The 4 least significant bits go to
     * the foreground color, the 3 bits following the background color and the
     * last bite to blink.
     */
    term->screen.attribute = (fg & 0xF) | ((bg & 0xF) << 4);
    if (blink) {
        term->screen.attribute |= BLINK_ATTR;
    }
}

/**
 * Set the foreground color.
 *
 * @param color The color to set.
 */
void setForeground(struct Terminal* term, int color) {
    setAttribute(term, (term->screen.attribute >> 4) & 0x7, color, term->screen.attribute & BLINK_ATTR);
}

/**
 * Set the background color.
 *
 * @param color The color to set.
 */
void setBackground(struct Terminal* term, int color) {
    setAttribute(term, color, term->screen.attribute & 0xF, term->screen.attribute & BLINK_ATTR);
}

/**
 * Set the blink attribute.
 *
 * @param color Whether to blink or not.
 */
void setBlink(struct Terminal* term, int blink) {
    setAttribute(term, (term->screen.attribute >> 4) & 0x7, term->screen.attribute & 0xF, blink);
}

/**
 * Set the current position to a character, and increase the cursor.
 *
 * This write to both the buffer and the video memory.
 * The attribute correspoding to the position is set to the global attribute.
 *
 * @param c The character to set.
 */
void setCharacter(struct Terminal* term, char c) {

    struct ScreenStatus* status = &term->screen;

    status->writableBuffer[2*status->cursorPosition] = c;
    status->writableBuffer[2*status->cursorPosition + 1] = status->attribute;

    change_character(term, status->cursorPosition, c, status->attribute);

    status->cursorPosition++;
}

/**
 * Scroll the screen delta lines.
 *
 * @param delta A natural number indicating the number of lines to scroll.
 */
void scrollLine(struct Terminal* term, int delta) {

    struct ScreenStatus* status = &term->screen;

    if (delta > BUFFER_ROWS) {
        delta = BUFFER_ROWS;
    } else if (delta < 1) {
        delta = 1;
    }

    status->lastLine -= delta;
    if (status->lastLine < 0) {
        status->lastLine = 0;
    }

    int i, j;
    // Move the video buffer up one line
    for (j = 0, i = delta * 2 * LINE_WIDTH; i < 2 * LINE_WIDTH * BUFFER_ROWS; i++, j++) {
        status->videoBuffer[j] = status->videoBuffer[i];
    }

    // Clear the lines that scrolled up
    for (i = LINE_WIDTH * (BUFFER_ROWS - delta); i < LINE_WIDTH * BUFFER_ROWS; i++) {
        status->videoBuffer[2 * i] = ' ';
        status->videoBuffer[2 * i + 1] = status->attribute;
    }

    // Reflect the changes to screen, if we're active
    flip_buffer(term);
}

/**
 * Erase the characters in the current line.
 *
 * The characters to be erased are determined by type. It can take:
 * - ERASE_ALL: Delete all characters in the current line.
 * - ERASE_LEFT: Erase all characters left of the cursor.
 * - ERASE_RIGHT: Erase all characters right of the cursor.
 *
 * @param type The type of erase to perform.
 */
void eraseLine(struct Terminal* term, int type) {

    int start, end, line = term->screen.cursorPosition / LINE_WIDTH;
    switch (type) {
        case ERASE_ALL:
            start = line * LINE_WIDTH;
            end = (line + 1) * LINE_WIDTH;
            break;
        case ERASE_LEFT:
            start = line * LINE_WIDTH;
            end = term->screen.cursorPosition;
            break;
        case ERASE_RIGHT:
        default:
            start = term->screen.cursorPosition + 1;
            end = (line + 1) * LINE_WIDTH;
            break;
    }

    setBlank(term, start, end);
}

/**
 * Clear part of the screen.
 *
 * The part of the screen of the be cleared is determined by type, it can take:
 * - CLEAR_ALL: Clear all of the screen
 * - CLEAR_ABOVE: Clear all the lines above, including the current.
 * - CLEAR_BELOW: Clear all the lines below.
 *
 *  @param type The part of the screen to clear.
 */
void clearScreen(struct Terminal* term, int type) {

    int start, end;
    switch (type) {
        case CLEAR_ALL:
            start = 0;
            end = TOTAL_ROWS * LINE_WIDTH;
            break;
        case CLEAR_ABOVE:
            start = 0;
            end = LINE_WIDTH * (term->screen.cursorPosition / LINE_WIDTH);
            break;
        case CLEAR_BELOW:
        default:
            start = (1 + term->screen.cursorPosition / LINE_WIDTH) * LINE_WIDTH;
            end = TOTAL_ROWS * LINE_WIDTH;
            break;
    }

    setBlank(term, start, end);
}

/**
 * Set the part of the screen between start and end to ' '.
 *
 * @param start The start position.
 * @parma end The end position.
 */
void setBlank(struct Terminal* term, int start, int end) {

    struct ScreenStatus* status = &term->screen;

    while (start < end) {
        change_character(term, start, ' ', status->attribute);

        status->writableBuffer[2*start] = ' ';
        status->writableBuffer[2*start + 1] = status->attribute;
        start++;
    }
}

/**
 * Parse the control buffer data.
 *
 * @param a A pointer to the place where the 1st int parsed will be stored.
 * @param b A pointer to the place where the 2nd int parsed will be stored.
 * @param def A default value to set if no value can be parsed.
 */
void parseControlBuffer(struct Terminal* term, int* a, int* b, int def) {

    struct ScreenStatus* status = &term->screen;

    // Set the null at the end, and then parse it
    if (status->controlBufferPos) {

        int i = 0;
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

/**
 * Write a string of bytes to screen.
 *
 * This function parses some control sequences that modifiy the screen status.
 * These start with "\e[" and are followed by a series of parameters.
 *
 * {@see handleControlSequence}
 *
 * @param buf The buffer from which to read the string of bytes.
 * @param length the number of bytes to read.
 *
 * @return The number of bytes written to screen.
 */
size_t tty_write(const void* buf, size_t length) {
    return tty_write_to_screen(tty_current(), buf, length);
}

size_t tty_write_active(const char* buf, size_t length) {
    return tty_write_to_screen(tty_active(), buf, length);
}

size_t tty_write_to_terminal(int terminal, const char* buf, size_t length) {
    return tty_write_to_screen(tty_terminal(terminal), buf, length);
}

size_t tty_write_to_screen(struct Terminal* term, const void* buf, size_t length) {

    struct ScreenStatus* status = &term->screen;
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
                            setCharacter(term, ' ');
                        }
                        break;
                    case '\v':
                        status->cursorPosition += LINE_WIDTH;
                        break;
                    case '\b':
                        if (status->cursorPosition > 0) {
                            status->cursorPosition -= 1;
                            setCharacter(term, ' ');
                            status->cursorPosition -= 1;
                        }
                        break;
                    default:
                        setCharacter(term, cur);
                        break;
                }

                if (status->cursorPosition >= LINE_WIDTH * TOTAL_ROWS) {
                    scrollLine(term, 1);
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
                    handleControlSequence(term, cur);
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

    update_cursor(term);

    return length;
}

/**
 * Handle a control sequence input.
 *
 * This handles all characters after a control sequence indicator "\e[".
 * When the sequence is finished, is determined by if the character added
 * is not recognised by this function.
 *
 * The control sequences handled are (n and m are used to indicate optional
 *  numeric arguments, all positions start at 1, not 0):
 * - "\e[nA" Move the cursor up n times. Defaults to 1.
 * - "\e[nB" Move the cursor down n times. Defaults to 1.
 * - "\e[nC" Move the cursor right n times. Defaults to 1.
 * - "\e[nD" Move the cursor left n times. Defaults to 1.
 * - "\e[nE" Move the cursor n lines down. Defaults to 1.
 * - "\e[nF" Move the cursor n lines up. Defaults to 1.
 * - "\e[nG" Jump the cursor to column n in the current row. Defaults to 1.
 * - "\e[n;mH" Jump the cursor to row n, column m in the screen. Default is 1,1.
 * - "\e[nJ" Clear the screen, see {@link clearScreen} for params.
 * - "\e[nK" Erase the current line, see {@link eraseLine} for params.
 * - "\e[nD" Jump to row n in the current column. Defaults to 1.
 * - "\e[nz" Takes any number of numerical arguments separated by ; and prints
 *   them in the bottom left corner of the screen.
 * - "\e[nm" Change the current attribute, see {@link changeColor}.
 *
 * @param cur An input character to the control sequence.
 */
void handleControlSequence(struct Terminal* term, char cur) {

    struct ScreenStatus* status = &term->screen;

    int mod1, mod2;
    switch (cur) {
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
            clearScreen(term, mod1);

            endControlSequence();
            break;
        case 'K':
            // Erase to (left|right|all)
            readControlBuffer(0);
            eraseLine(term, mod1);

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
            mod1 = status->cursorPosition;
            status->cursorPosition = (TOTAL_ROWS - 1) * LINE_WIDTH;
            mod2 = 0;
            while (status->controlBufferPos > mod2) {
                setCharacter(term, status->controlBuffer[mod2++]);
            }
            status->cursorPosition = mod1;
            endControlSequence();
            break;
        case 'm':
            // Change color!
            readControlBuffer(0);
            changeColor(term, mod1);
            endControlSequence();
            break;
        default:
            endControlSequence();
            break;
    }
}

/**
 * Change the current screen color as needed by the "\e[nm" sequence.
 *
 * The numerical argument determins the color to be changed, and to what.
 * It can take one of the following values:
 * - 1: Make the font bold.
 * - 22: Disable bold.
 * - 37: Set the background white.
 * - 36: Set the background cyan.
 * - 35: Set the background magenta.
 * - 34: Set the background blue.
 * - 33: Set the background green.
 * - 32: Set the background red.
 * - 31: Set the background black.
 * - 39: Set the background black.
 * - 49: Set the foreground white.
 * - 47: Set the foreground white.
 * - 46: Set the foreground cyan.
 * - 45: Set the foreground magenta.
 * - 44: Set the foreground blue.
 * - 43: Set the foreground yellow.
 * - 42: Set the foreground green.
 * - 41: Set the foreground red.
 * - 40: Set the foreground black.
 * - 25: Disable blink.
 * - 5: Enable blink.
 * - 0: Reset the colors to default.
 *
 * @param mod1 The numerical argument passed to the sequence.
 */
void changeColor(struct Terminal* term, int mod1) {

    struct ScreenStatus* status = &term->screen;

    int mod2;
    switch (mod1) {
        case 1:
            // Bold
            // Isolate the foreground color to check for boldness
            mod2 = status->attribute & 0xF;
            if (mod2 != COLOR_BLACK && mod2 != COLOR_BROWN && mod2 < COLOR_GRAY) {
                setForeground(term, mod2 + 0x8);
            }
            break;
        case 22:
            // Not bold
            // Isolate the foreground color to check for boldness
            mod2 = status->attribute & 0xF;
            if (mod2 >= COLOR_BRIGHT_BLUE && mod2 != COLOR_YELLOW) {
                setForeground(term, mod2 - 0x8);
            }
            break;
        case 37:
            // White bg
            setBackground(term, COLOR_WHITE);
            break;
        case 36:
            // Cyan bg
            setBackground(term, COLOR_CYAN);
            break;
        case 35:
            // Magenta bg
            setBackground(term, COLOR_MAGENTA);
            break;
        case 34:
            // Blue bg
            setBackground(term, COLOR_BLUE);
            break;
        case 32:
            // Green bg
            setBackground(term, COLOR_GREEN);
            break;
        case 31:
            // Red bg
            setBackground(term, COLOR_RED);
            break;
        case 39:
        case 30:
            // Black bg
            setBackground(term, COLOR_BLACK);
            break;
        case 49:
        case 47:
            // White fg
            setForeground(term, COLOR_WHITE);
            break;
        case 46:
            // Cyan fg
            setForeground(term, COLOR_CYAN);
            break;
        case 45:
            // Magenta fg
            setForeground(term, COLOR_MAGENTA);
            break;
        case 44:
            // Blue fg
            setForeground(term, COLOR_BLUE);
            break;
        case 43:
            // Yellow fg
            setForeground(term, COLOR_YELLOW);
            break;
        case 42:
            // Green fg
            setForeground(term, COLOR_GREEN);
            break;
        case 41:
            // Red fg
            setForeground(term, COLOR_RED);
            break;
        case 40:
            // Black fg
            setForeground(term, COLOR_BLACK);
            break;
        case 25:
            // Time of the Angels
            // If you blink, you die
            setBlink(term, 0);
            break;
        case 5:
            // Blink!
            setBlink(term, 1);
            break;
        case 0:
            // Reset
            setBlink(term, 0);
            setForeground(term, COLOR_BLACK);
            setBackground(term, COLOR_WHITE);
            break;
    }
}

