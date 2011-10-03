#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/call.h"
#include "library/time.h"
#include "mcurses/mcurses.h"
#include "system/call/codes.h"
#include "system/call/ioctl/keyboard.h"
#include "type.h"

void top(char* arv) {

    static termios oldStatus;
    static termios topStatus = { 0, 0 };

    ioctl(0, TCGETS, (void*) &oldStatus);
    ioctl(0, TCSETS, (void*) &topStatus);
    
    // Reset the screen
    moveCursor(1, 1);
    clearScreen(CLEAR_ALL);

    struct ProcessInfo data[20];
    int pcount = system_call(_SYS_PINFO, data, 20,0);
    
    printf("PID\tPPID\tUSER\t%%CPU\tPRIO\tSTATE\tSTART DATE\n");

    do {
        moveCursor(2, 1);
        for (int i = 0; i < pcount; i++) {
            printf("%d\t%d\t",data[i].pid, data[i].ppid);
            printf("%s\t", "acrespo");
            printf("%d\t", data[i].cputime);
            printf("%d\t%s\t",data[i].priority, (data[i].state)? "alive":  "zombie");
            printf("%s",asctime(localtime(&data[i].timeStart)));
        }

    } while(getchar() != 'q');

    clearScreen(CLEAR_ALL);
    moveCursor(0, 0);
    ioctl(0, TCSETS, (void*) &oldStatus);

}



void manTop(void) {
    setBold(1);
    printf("Usage:\n\t top\n");
    setBold(0);
}
