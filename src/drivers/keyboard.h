#ifndef _drivers_keyboard_header
#define _drivers_keyboard_header

#include "type.h"
#include "system/process/process.h"

void keyboard_read(void);

void keyboard_leds(int caps, int num, int scroll);

unsigned char keyboard_get_code(void);

void keyboard_consumer(struct Process* p);

#endif
