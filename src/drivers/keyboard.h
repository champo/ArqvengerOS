#ifndef _drivers_keyboard_header
#define _drivers_keyboard_header

#include "type.h"

void readScanCode();

size_t readKeyboard(void* buffer, size_t count);

#endif
