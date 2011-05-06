#include "../include/type.h"
#include "../include/video.h"
#include "../include/io.h"

void updateCursor();

static int cursorPosition = 0;


void write(const char* str, size_t length) {
    
    char* video = (char*) 0xb8000;
    int aux = 0;    
    while (length--) {
        video[cursorPosition * 2] = str[aux++];
        cursorPosition++;
    }
    updateCursor();
    
    if (cursorPosition >= 80*25 ) {
        cursorPosition = 0;
    }
}

void moveCursor(int row, int col) {

    cursorPosition=(row*80) + col;
    updateCursor();
}

void updateCursor(){

    // cursor LOW port to vga INDEX register
    outB(0x3D4, 0x0F);
    outB(0x3D5, (unsigned char)(cursorPosition&0xFF));
    // cursor HIGH port to vga INDEX register
    outB(0x3D4, 0x0E);
    outB(0x3D5, (unsigned char )((cursorPosition>>8)&0xFF));
 }
