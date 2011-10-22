#include "shell/echo/echo.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/string.h"
#include "mcurses/mcurses.h"


/**
 * No TP is done without the ROFLcopter.
 *
 * @param argv A string containg everything that came after the command.
 */
void roflcopter(char* argv) {
    printf("ROFL:ROFL:LOL:ROFL:ROFL\n"
           "           |\n"
           "  L   /---------\n"
           " LOL===       []\\\n"
           "  L    \\         \\\n" 
           "        \\_________\\\n"
           "          |     |\n"
           "       -------------/\n\n");
}

/**
 * Print manual page in case you don't understand.
 */
void manRoflcopter(void) {
    setBold(1);
    printf("Usage:\n\t roflcopter");
    setBold(0);

    printf("\n\n \t\"We should totally put a ROFLcopter in our OS.\"\n\n");
}

