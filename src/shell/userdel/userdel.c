#include "shell/userdel/userdel.h"
#include "system/accessControlList/users.h"
#include "system/accessControlList/groups.h"
#include "library/string.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"


void userdel(char* argv) {
    
    char* firstSpace = strchr(argv, ' ');
    struct User* user;

    if (firstSpace == NULL) {
        printf("Usage: userdel LOGIN\n");
    } else if ((user = get_user_by_name(firstSpace + 1)) == NULL) {
        printf("userdel: user '%s' does not exists.\n", firstSpace + 1) ;
    } else {

        delete_group_member(user->gid[0], user->id);
        delete_user(firstSpace + 1);

        free_user(user);

        printf("Successfully deleted user account.\n");
    }
}


void manUserdel(void) {
   setBold(1);
   printf("Usage:\n\tuserdel");
   setBold(0);

   printf(" LOGIN\n\n");
}
