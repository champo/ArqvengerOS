#include "shell/stacksize/stacksize.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/string.h"
#include "library/ctype.h"
#include "mcurses/mcurses.h"
#include "system/mm/page.h"

static void growStack(int step, unsigned int size);

void stacksize(char* argv) {

    unsigned int size = (MAX_PAGES_IN_STACK + 1) * PAGE_SIZE;
    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd != NULL) {
        cmdEnd++;
        if (!isdigit(cmdEnd[0])) {
            printf("Invalid arguments.\n");
            return;
        }
        size = atoi(cmdEnd);
    }
    growStack(0, size);
}

void manstacksize(void) {
    setBold(1);
    printf("Usage:\n\t stacksize");
    setBold(0);

    printf(" [SIZE]\n");
}

void growStack(int step, unsigned int size) {
    if (processStackSize() >= size) {
        if (step > 0) {
            printf("Process ended correctly.\n");
        } else {
            printf("The size of the stack was bigger than it was intended when the process began.\n");
        }
        return;
    }
    step++;
    growStack(step, size);
}
