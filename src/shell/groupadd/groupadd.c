#include "shell/groupadd/groupadd.h"
#include "system/accessControlList/groups.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "library/string.h"


void groupadd(char* argv) {
    char* firstSpace = strchr(argv, ' ');
    if (firstSpace == NULL) {
        printf("Usage: groupadd GROUP\n"); 
        return;
    }

    create_group(firstSpace);
}

void manGroupadd(void) {
    setBold(1);
    printf("Usage:\n\t groupadd ");
    setBold(0);

    printf("GROUP\n");
}
