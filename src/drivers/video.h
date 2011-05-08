#ifndef _drivers_video_header
#define _drivers_video_header

#include "type.h"

void write(const char* str,size_t length);

void moveCursor(int row,int col);

void clearScreen();

#endif
