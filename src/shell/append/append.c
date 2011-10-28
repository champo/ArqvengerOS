#include "shell/append/append.h"
#include "library/stdio.h"
#include "constants.h"
#include "library/stdlib.h"
#include "library/string.h"
#include "system/call/ioctl/keyboard.h"
#include "mcurses/mcurses.h"

static termios status = { .canon = 0, .echo = 1 };

void append(char* argv) {

    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd == NULL) {
        printf("Usage: append path\n");
        return;
    }

    termios oldStatus;
    ioctl(0, TCGETS, (void*) &oldStatus);
    ioctl(0, TCSETS, (void*) &status);

    cmdEnd++;
    int fd = open(cmdEnd, O_APPEND | O_WRONLY | O_CREAT, 0777);
    char c;
    while ((c = getchar()) != '\033') {

        if (write(fd, &c, 1) != 1) {
            printf("\nCant write to the file. Sorry!");
            break;
        }
    }
    printf("\n");

    ioctl(0, TCSETS, (void*) &oldStatus);
}

void man_append(void) {
    setBold(1);
    printf("Usage:\n\t append");
    setBold(0);

    printf(" file\n");

    printf("Write the contents of stdin until the ESC key is pressed.\n");
}
