#include "shell/date/date.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "mcurses/mcurses.h"
#include "library/time.h"


/**
 * Command that shows the present date and current local time.
 *
 * @param argv A string containing everything that came after the command.
 */
void date(char* argv) {

    UNUSED(argv);

    time_t rawTime = time(NULL);
    struct tm* timeInfo = localtime(&rawTime);
    char *str = asctime(timeInfo);
    printf("%s \n",str);
}

/**
 * Print manual page for the date command.
 *
 */
void manDate(void) {
    setBold(1);
    printf("Usage:\n\tdate\n");
    setBold(0);
}
