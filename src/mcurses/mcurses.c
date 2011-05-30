#include "mcurses/mcurses.h"
#include "library/stdio.h"

/**
 * Move the cursor to a point in the screen.
 *
 * @param r The row to jump to.
 * @param c The column to jump to.
 */
void moveCursor(int r, int c) {
    printf("\033[%d;%dH", r, c);
}

/**
 * Jump to a column while staying on the same row.
 *
 * @param c The column to jump to.
 */
void moveCursorInRow(int c) {
    printf("\033[%dG", c);
}

/**
 * Jump to a row while staying on the same column.
 *
 * @param r The row to jump to.
 */
void moveCursorInColumn(int r) {
    printf("\033[%dd", r);
}

/**
 * Clear part the of the screen.
 *
 * type can take one of three values.
 * - CLEAR_ALL: Clear the whole screen.
 * - CLEAR_BELOW: Clear the lines below the current.
 * - CLEAR_ABOVE: Clear the lines above the current.
 *
 * @param type The part of the screen to clear.
 */
void clearScreen(int type) {
    if (type != CLEAR_BELOW && type != CLEAR_ALL && type != CLEAR_ABOVE) {
        type = CLEAR_BELOW;
    }

    printf("\033[%dJ", type);
}

/**
 * Clear part of the current line.
 *
 * type can take one of three values.
 * - ERASE_ALL: Erase the whole line.
 * - ERASE_RIGHT: Erase the characters right of the cursor.
 * - ERASE_LEFT: Erase the characters left of the cursor.
 *
 * @param type The part of the line to clear.
 */
void clearLine(int type) {
    if (type != ERASE_RIGHT && type != ERASE_ALL && type != ERASE_LEFT) {
        type = ERASE_RIGHT;
    }

    printf("\033[%dK", type);
}

/**
 * Set the background color.
 *
 * @param color The color to set, one of the COLOR_* constants.
 */
void setBackgroundColor(int color) {

    int c;
    switch (color) {
        case COLOR_BLACK:
            c = 0;
            break;
        case COLOR_BLUE:
            c = 4;
            break;
        case COLOR_RED:
            c = 1;
            break;
        case COLOR_GREEN:
            c = 2;
            break;
        case COLOR_MAGENTA:
            c = 5;
            break;
        case COLOR_CYAN:
            c = 6;
            break;
        default:
        case COLOR_WHITE:
            c = 7;
            break;
    }

    printf("\033[3%dm", c);
}

/**
 * Set the foreground color.
 *
 * @param color The color to set, one of the COLOR_* constants.
 */
void setForegroundColor(int color) {

    int c;
    switch (color) {
        case COLOR_BLACK:
            c = 0;
            break;
        case COLOR_BLUE:
            c = 4;
            break;
        case COLOR_RED:
            c = 1;
            break;
        case COLOR_GREEN:
            c = 2;
            break;
        case COLOR_MAGENTA:
            c = 5;
            break;
        case COLOR_CYAN:
            c = 6;
            break;
        case COLOR_YELLOW:
            c = 3;
            break;
        default:
        case COLOR_WHITE:
            c = 7;
            break;
    }

    printf("\033[4%dm", c);
}

/**
 * Set the blink flag.
 *
 * @param blink True to make it blink, False to stop the blinking.
 */
void setBlink(int blink) {
    printf("\033[%dm", blink ? 5 : 25);
}

/**
 * Set the bold flag.
 *
 * @param bold True to make it bold, False to make it not bold.
 */
void setBold(int bold) {
    if (bold) {
        printf("\033[1m");
    } else {
        printf("\033[22m");
    }
}

