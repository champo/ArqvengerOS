#include "type.h"
#include "drivers/video.h"
#include "drivers/videoControl.h"
#include "system/io.h"

static char* videoMemory = (char*) 0xb8000;

/**
 * Draw the current buffer to screen.
 */
void video_flip_buffer(const char* buffer) {

    for (int i = 0; i < 2 * LINE_WIDTH * TOTAL_ROWS; i++) {
        videoMemory[i] = buffer[i];
    }
}

/**
 * Update the video cursor position to match the current logic position.
 */
void video_update_cursor(int pos) {

    // cursor LOW port to vga INDEX register
    outB(0x3D4, 0x0F);
    outB(0x3D5, (unsigned char)(pos & 0xFF));

    // cursor HIGH port to vga INDEX register
    outB(0x3D4, 0x0E);
    outB(0x3D5, (unsigned char )((pos >> 8) & 0xFF));
}

void video_set_char(int pos, char value, char attribute) {
    videoMemory[2 * pos] = value;
    videoMemory[2 * pos + 1] = attribute;
}

