#include "system/tick.h"
#include "common.h"
#include "library/stdio.h"
#include "library/string.h"

size_t ticksSinceStart = 0;

void timerTick() {
    ticksSinceStart++;
    char *original = "holaoa";
    puts(strrchr(original,'o'));
    
    //putchar('a');
    //putchar('\n');
    while(1){}
}
