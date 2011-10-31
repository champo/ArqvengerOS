#include "shell/busywait/busywait.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"


void busywait(char* argv) {
    
    printf("I'm busywaiting...\n");

    while(1);
}

void manBusywait(void) {
    setBold(1);
    printf("Usage:\n\t busywait\n");
    setBold(0);
}
