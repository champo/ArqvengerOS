#ifndef _drivers_video_header
#define _drivers_video_header

#include "type.h"

size_t writeScreen(const void* buf, size_t length);

void initScreen(void);

#endif
