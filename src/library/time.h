#ifndef __library_time_header__
#define __library_time_header__

#include "type.h"

struct tm {
    int sec;
    int min;
    int hour;
    int mday;
    int mon;
    int year;
    int wday;
    int yday;
    int isdst;
};

time_t time(time_t *tp);

char* asctime(const struct tm *tp);

struct tm* localtime(const time_t* timer);

#endif

