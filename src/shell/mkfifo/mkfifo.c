#include "shell/mkfifo/mkfifo.h"
#include "library/stdio.h"
#include "constants.h"
#include "library/stdlib.h"
#include "library/string.h"
#include "mcurses/mcurses.h"

void command_mkfifo(char* argv) {

    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd == NULL) {
        printf("Usage: mkfifo path\n");
        return;
    }

    cmdEnd++;
    switch (mkfifo(cmdEnd)) {
        case -1:
            printf("Cant create '%s': No such file or directory.\n", cmdEnd);
            break;
        case EEXIST:
            printf("Cant create '%s': File already exists.\n", cmdEnd);
            break;
        case EIO:
            printf("Cant create '%s': Some wizardry went awry. Try again later.\n", cmdEnd);
            break;
    }
}

void manFifo(void) {
    setBold(1);
    printf("Usage:\n\tmkfifo ");
    setBold(0);
    printf("NAME\n");
}

