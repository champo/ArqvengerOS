#include "shell/top/top.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/time.h"
#include "library/sys.h"
#include "system/call/codes.h"
#include "system/call/ioctl/keyboard.h"
#include "system/accessControlList/users.h"
#include "system/accessControlList/groups.h"
#include "type.h"
#include "mcurses/mcurses.h"

void top(char* argv) {

    struct ProcessInfo data[20];
    int pcount;

    termios oldStatus;
    termios topStatus = { 0, 0 };

    ioctl(0, TCGETS, (void*) &oldStatus);
    ioctl(0, TCSETS, (void*) &topStatus);

    // Reset the screen
    moveCursor(1, 1);
    clearScreen(CLEAR_ALL);


    printf("PID   PPID  USER\t\t  GROUP\t\t %%CPU   PRIO STATE  COMMAND\n");

    do {
        moveCursor(2, 1);
        clearScreen(CLEAR_BELOW);

        pcount = pinfo(data, 20);

        for (int i = 0; i < pcount; i++) {
            printf("%2d\t", data[i].pid);
            printf("%2d\t", data[i].ppid);
            printf("%10s\t", get_username(data[i].uid));
            printf("%10s\t", get_groupname(data[i].gid));
            printf("%3d\t", data[i].cputime);
            printf("%1d\t", data[i].priority);
            printf("%6s ", (data[i].state)? "alive":  "zombie");
            //printf("%s\t", asctime(localtime(&data[i].timeStart)));
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
