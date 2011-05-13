#ifndef _system_call_keyboard_header_
#define _system_call_keyboard_header_

#include "type.h"

#define TCGETS 0x00005401
#define TCSETS 0x00005402

typedef struct {
    byte canon;
    byte echo;
} termios;

#endif
