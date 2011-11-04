#ifndef __SYSTEM_KPRINTF__
#define __SYSTEM_KPRINTF__

#define LOG_DEBUG 3
#define LOG_INFO 2
#define LOG_ERROR 1
#define LOG_QUIET 0

int kprintf(const char* format, ...);

int log_debug(const char* format, ...);

int log_info(const char* format, ...);

int log_error(const char* format, ...);

void set_log_level(int level);

#endif
