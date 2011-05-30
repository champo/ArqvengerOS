#include "mcurses/mcurses.h"
#include "library/stdio.h"

void moveCursor(int r, int c) {
    printf("\033[%d;%dH", r, c);
}

void moveCursorInRow(int c) {
    printf("\033[%dG", c);
}

void moveCursorInColumn(int r) {
    printf("\033[%dd", r);
}

void clearScreen(int type) {
    if (type != CLEAR_BELOW && type != CLEAR_ALL && type != CLEAR_ABOVE) {
        type = CLEAR_BELOW;
    }

    printf("\033[%dJ", type);
}

void clearLine(int type) {
    if (type != ERASE_RIGHT && type != ERASE_ALL && type != ERASE_LEFT) {
        type = ERASE_RIGHT;
    }

    printf("\033[%dK", type);
}

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

void setBlink(int blink) {
    printf("\033[%dm", blink ? 5 : 25);
}

void setBold(int bold) {
    if (bold) {
        printf("\033[1m");
    } else {
        printf("\033[22m");
    }
}

