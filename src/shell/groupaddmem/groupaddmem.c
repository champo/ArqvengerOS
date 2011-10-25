#include "shell/groupaddmem/groupaddmem.h"
#include "system/accessControlList/groups.h"
#include "system/accessControlList/users.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "library/string.h"



void groupaddmem(char* argv) {

    char* firstSpace = strchr(argv,' ');
    if (firstSpace == NULL) {
        printf("Usage: groupaddmem GROUP MEMBER\n");
        return;
    }
    char* secondSpace = strchr(firstSpace, ' ');

    if (get_group_by_name(firstSpace) == NULL) {
        return ;
    }

}


void manGroupaddmem(void) {
    setBold(1);
    printf("Usage:\n\t groupaddmem ");
    setBold(0);

    printf("GROUP MEMBER\n");
}

