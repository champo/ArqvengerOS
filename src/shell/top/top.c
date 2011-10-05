#include "shell/top/top.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/time.h"
#include "library/sys.h"
#include "system/call/codes.h"
#include "system/call/ioctl/keyboard.h"
#include "type.h"
#include "mcurses/mcurses.h"

void top(char* argv) {

    static termios oldStatus;
    static termios topStatus = { 0, 0 };

    ioctl(0, TCGETS, (void*) &oldStatus);
    ioctl(0, TCSETS, (void*) &topStatus);
    
    // Reset the screen
    moveCursor(1, 1);
    clearScreen(CLEAR_ALL);

    struct ProcessInfo data[20];
    int pcount = pinfo(data, 20);
  
    printf("PID\tPPID\tUSER\tGROUP\t%%CPU\tPRIO\tSTATE\tSTART DATE\tCOMMAND\n");
    
    do {
        moveCursor(2, 1);
        for (int i = 0; i < pcount; i++) {
            printf("%d\t", data[i].pid);
            printf("%d\t", data[i].ppid);
            printf("%d\t", data[i].uid);
            printf("%d\t", data[i].gid);
            printf("%d\t", data[i].cputime);
            printf("%d\t", data[i].priority);
            printf("%s\t", (data[i].state)? "alive":  "zombie");
            printf("%s\t", asctime(localtime(&data[i].timeStart)));
            printf("%s", data[i].name);
            printf("\n");
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
