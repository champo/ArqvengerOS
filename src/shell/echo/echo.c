#include "shell/echo/echo.h"
#include "library/stdio.h"
#include "library/string.h"

void echo(char* argv) {

    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd != NULL) {
        cmdEnd++;
        printf("%s\n", cmdEnd);
    }
}
