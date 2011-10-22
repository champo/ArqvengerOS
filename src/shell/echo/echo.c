#include "shell/echo/echo.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/string.h"
#include "mcurses/mcurses.h"


/**
 * Command that makes an echo, meaning it prints everything it receives as a parameter.
 *
 * @param argv A string containg everything that came after the command.
 */
void echo(char* argv) {

    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd != NULL) {
        cmdEnd++;
        printf("%s\n", cmdEnd);
    }
}

/**
 * Print manual page for the echo command.
 */
void manEcho(void) {
    setBold(1);
    printf("Usage:\n\t echo");
    setBold(0);

    printf(" [string ...]\n");
}
