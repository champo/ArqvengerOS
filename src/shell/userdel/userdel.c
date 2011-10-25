#include "shell/userdel/userdel.h"
#include "library/string.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"


void userdel(char* argv) {
    
    char* firstSpace = strchr(argv, ' ');

    if (firstSpace == NULL) {
        printf("Usage: userdel LOGIN\n");
    } else if (get_user_by_name(firstSpace) == NULL) {
        printf("userdel: user '%s' does not exists.\n", firstSpace);
    } else {
        delete_user(firstSpace);
    }
}


void manUserdel(void) {
   setBold(1);
   printf("Usage:\n\tuserdel");
   setBold(0);

   printf(" LOGIN\n\n");
}
