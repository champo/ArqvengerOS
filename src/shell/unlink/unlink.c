#include "shell/unlink/unlink.h"
#include "library/stdio.h"
#include "constants.h"
#include "library/stdlib.h"
#include "library/string.h"

void command_unlink(char* argv) {

    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd == NULL) {
        printf("Usage: unlink path\n");
        return;
    }

    cmdEnd++;
    switch (unlink(cmdEnd)) {
        case -1:
        case ENOENT:
            printf("Cant remove '%s': No such file or directory.\n", cmdEnd);
            break;
        case EIO:
            printf("Cant remove '%s': Some wizardry went awry. Try again later.\n", cmdEnd);
            break;
        case EISDIR:
            printf("Cant remove '%s': It's a directory. Try rmdir.\n", cmdEnd);
            break;
    }
}

void manUnlink(void) {
    setBold(1);
    printf("Usage:\n\tunlink ");
    setBold(0);
    printf("FILE\n");


}
