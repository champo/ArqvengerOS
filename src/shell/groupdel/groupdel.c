#include "shell/groupdel/groupdel.h"
#include "system/accessControlList/groups.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "library/string.h"

void groupdel(char* argv) {

    char* firstSpace = strchr(argv, ' ');
    if (firstSpace == NULL) {
        printf("Usage: groupdel GROUP\n");
    } else if (get_group_by_name(firstSpace + 1) == NULL) {
        printf("groupdel: group '%s' does not exists.\n", firstSpace);
    } else {
        delete_group(firstSpace + 1);
    }
}


void manGroupdel(void) {
    setBold(1);
    printf("Usage:\n\t groupdel ");
    setBold(0);
    
    printf("GROUP\n");

    //IS THIS TOO MUCH???
    printf("\n");
    setBold(1);
    printf("DESCRIPTION\n");
    setBold(0);

    printf("The groupdel command modifies the system account files, deleting \
            all entries that refer to group. The named group must exist.\n");

    setBold(1);
    printf("CAVEATS\n");
    setBold(0);

    printf("You may not remove the primary group of any existing user. You \
            must remove the user before you remove the group.\n");
    printf("You should manually check all file systems to ensure that no \
            files remain owned by this group.\n");



}
