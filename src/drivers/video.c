#include "type.h"
#include "drivers/video.h"
#include "system/io.h"

/* Video attribute. White letters on black background. */
#define WHITE_TXT 0x07

void updateCursor();

static int cursorPosition = 0;

void clearScreen() {

    char *vidmem = (char *) 0xb8000;
    unsigned int i=0;

    while(i < (80*25*2)) {
        vidmem[i]=' ';
        i++;
        vidmem[i]=WHITE_TXT;
        i++;
    }
}

size_t writeScreen(const void* buf, size_t length) {

    char* video = (char*) 0xb8000;
    const char* str = (const char*) buf;

    size_t aux = 0;

    while (length--) {
        video[cursorPosition * 2] = str[aux++];
        cursorPosition++;

        if (cursorPosition >= 80*25 ) {
            cursorPosition = 0;
        }
    }

    updateCursor();

    return aux;
}

void moveCursor(int row, int col) {
    cursorPosition=(row*80) + col;
    updateCursor();
}

void updateCursor() {
    // cursor LOW port to vga INDEX register
    outB(0x3D4, 0x0F);
    outB(0x3D5, (unsigned char)(cursorPosition&0xFF));

    // cursor HIGH port to vga INDEX register
    outB(0x3D4, 0x0E);
    outB(0x3D5, (unsigned char )((cursorPosition>>8)&0xFF));
 }
