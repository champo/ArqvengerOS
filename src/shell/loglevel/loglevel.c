#include "shell/loglevel/loglevel.h"
#include "library/stdio.h"
#include "library/string.h"
#include "mcurses/mcurses.h"
#include "system/kprintf.h"

void command_log(char* argv) {
    int level = -1;
    
    char* arg = strchr(argv, ' ');

    if (arg == NULL) {
        printf("Invalid arguments.\n");
        return;
    }

    arg++;

    if (strcmp(arg, "0") == 0) {
        level = 0;
    } else if (strcmp(arg, "1") == 0) {
        level = 1;
    } else if (strcmp(arg, "2") == 0) {
        level = 2;
    } else if (strcmp(arg, "3") == 0) {
        level = 3;
    } else {
        printf("Invalid arguments.\n");
        return;
    }

    loglevel(level);
}

void man_log(void) {
    setBold(1);
    printf("Usage:\n\t loglevel");
    setBold(0);

    printf(" LEVEL\n\n");
    printf("LEVEL can be a 0 to indicate quiet, 1 to only show errors, 2 to show info and 3 to turn on debugging comments.\n");

}


