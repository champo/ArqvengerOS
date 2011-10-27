#include "shell/userdel/userdel.h"
#include "library/string.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"


void userdel(char* argv) {
    
    char* firstSpace = strchr(argv, ' ');

    if (firstSpace == NULL) {
        printf("Usage: userdel LOGIN\n");
    } else if (get_user_by_name(firstSpace + 1) == NULL) {
        printf("userdel: user '%s' does not exists.\n", firstSpace + 1) ;
    } else {
        delete_user(firstSpace + 1);
    }
}


void manUserdel(void) {
   setBold(1);
   printf("Usage:\n\tuserdel");
   setBold(0);

   printf(" LOGIN\n\n");
}
