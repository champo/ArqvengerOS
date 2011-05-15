#ifndef _mcurses_mcurses_header_
#define _mcurses_mcurses_header_

#include "drivers/videoControl.h"

void moveCursor(int r, int c);

void moveCursorInRow(int c);

void moveCursorInColumn(int r);

void clearScreen(int type);

void clearLine(int type);

void setBackgroundColor(int color);

void setForegroundColor(int color);

void setBlink(int blink);

void setBold(int bold);

#endif
