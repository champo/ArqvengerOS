#include "shell/groupadd/groupadd.h"
#include "system/accessControlList/groups.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "library/string.h"


void groupadd(char* argv) {
    char* firstSpace = strchr(argv, ' ');
    if (firstSpace == NULL || strchr(firstSpace + 1, ' ') != NULL || strcmp(firstSpace + 1, "") == 0) {
        printf("Usage: groupadd GROUP\n"); 
        return;
    }
    
    if (create_group(firstSpace + 1) == -1) {
        printf("groupadd: could not create group\n");
        printf("groupadd: it seems as if maximum amount of groups have reached\n");
        printf("groupadd: try deleting some groups to make some room\n");
        return;
    }

    printf("Successfully created group.\n");
}

void manGroupadd(void) {
    setBold(1);
    printf("Usage:\n\t groupadd ");
    setBold(0);

    printf("GROUP\n");
}
