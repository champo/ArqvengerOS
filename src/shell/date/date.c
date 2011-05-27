#include "shell/date/date.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/string.h"
#include "mcurses/mcurses.h"
#include "system/rtc.h"
#include "library/time.h"

void date(char* argv) {
    
    char *str = asctime(NULL);
    printf("%s \n",str);
}

void manDate(void) {
    setBold(1);
    printf("Usage:\n\tdate\n");
    setBold(0);
}
