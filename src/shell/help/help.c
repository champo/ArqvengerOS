#include "shell/help/help.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "shell/info.h"

void help(char* argv) {

    const Command* commands;
    size_t numCommands, i;

    commands = getShellCommands(&numCommands);

    for (i = 0; i < numCommands; i++) {
        setBold(1);
        printf("%s", commands[i].name);
        setBold(0);
        printf(": %s\n", commands[i].desc);
    }
}

void manHelp(void) {
    setBold(1);
    printf("help\n\t");
    setBold(0);

    printf("Print the list of shell commands.\n\n");

    setBold(1);
    printf("Usage:\n\thelp\n");
    setBold(0);
}
