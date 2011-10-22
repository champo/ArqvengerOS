#include "shell/cat/cat.h"
#include "library/stdio.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/string.h"
#include "constants.h"
#include "mcurses/mcurses.h"

void cat(char* argv) {

    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd == NULL) {
        printf("cat needs at least one parameter\n");
        return;
    }

    cmdEnd++;
    printf("Got fd %d\n", open(cmdEnd, O_RDWR));
}

void manCat(void) {
    setBold(1);
    printf("Usage:\n\t cat");
    setBold(0);

    printf(" file\n");
}

