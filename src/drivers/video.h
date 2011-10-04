#ifndef _drivers_video_header
#define _drivers_video_header

void video_flip_buffer(const char* buffer);

void video_update_cursor(int pos);

void video_set_char(int pos, char value, char attribute);

#endif
