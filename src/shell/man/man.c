#include "shell/man/man.h"
#include "shell/info.h"
#include "library/string.h"
#include "library/stdlib.h"
#include "library/stdio.h"
#include "library/ctype.h"
#include "mcurses/mcurses.h"

int displayManual(char* name);

void man(char* argv) {
    char* firstSpace = strchr(argv, ' ');

    if (firstSpace == NULL || !displayManual(firstSpace + 1)) {
        printf("What manual page do you want?\n");
    }
}

int displayManual(char* name) {

    size_t i, len = strlen(name), nameLen, numCommands;
    char* realStart;
    char* nextSpace;
    const Command* commands;

    // Consume white space
    for (i = 0; i < len && isspace(name[i]); i++);
    if (i == len) {
        // The string is only white space, we've been lied to!
        return 0;
    }

    realStart = name + i;
    nextSpace = strchr(realStart, ' ');
    if (nextSpace != NULL) {
        nameLen = nextSpace - realStart;
    } else {
        nameLen = strlen(realStart);
    }

    commands = getShellCommands(&numCommands);
    for (i = 0; i < numCommands; i++) {

        if (strncmp(commands[i].name, realStart, nameLen) == 0) {

            setBold(1);
            printf("%s\n", commands[i].name);
            setBold(0);
            printf("\t%s\n\n", commands[i].desc);

            if (commands[i].man != NULL) {
                commands[i].man();
            }
            return 1;
        }
    }

    return 0;
}

void manMan(void) {
    setBold(1);
    printf("Usage:\n\tman");
    setBold(0);

    printf(" commandname\n\n");

    setBold(1);
    printf("Also see:\n\thelp");
    setBold(0);
    printf(" command for a list of available commands.\n");
}
