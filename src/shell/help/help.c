#include "shell/help/help.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "shell/info.h"
#include "system/call/ioctl/keyboard.h"

static termios shellStatus = { 0, 0 };

/**
 * Command that shows the commands supported by the shell, as well as a brief description.
 *
 * @param argv A string containing everything that came after the command.
 */
void help(char* argv) {

    UNUSED(argv);

    const Command* commands;
    size_t numCommands, i;

    commands = getShellCommands(&numCommands);

    termios inputStatus;
    ioctl(0, TCGETS, (void*) &inputStatus);
    ioctl(0, TCSETS, (void*) &shellStatus);

    printf("ProTip: You can use functions keys 1 through 4 to switch between shells.\n");

    setBold(1);
    printf("Commands:\n");
    setBold(0);

    size_t usableLines = TOTAL_ROWS - 2;

    for (i = 0; i < numCommands; i++) {

        if (i >= usableLines) {
            printf(" -- Press enter to see more (or q to quit) -- ");
            int c;
            while ((c = getchar()) != '\n') {
                if (c == 'q') {
                    printf("\n");
                    ioctl(0, TCSETS, (void*) &inputStatus);
                    return;
                }
            }

            moveCursorInRow(1);
            clearLine(ERASE_ALL);
        }

        setBold(1);
        printf("\t%s", commands[i].name);
        setBold(0);
        printf(": %s\n", commands[i].desc);
    }

    ioctl(0, TCSETS, (void*) &inputStatus);
}

/**
 * Print manual page for the help command.
 */
void manHelp(void) {
    setBold(1);
    printf("Usage:\n\thelp\n");
    setBold(0);
}
