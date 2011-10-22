#include "shell/users/users.h"
#include "mcurses/mcurses.h"
#include "system/accessControlList/users.h"
#include "library/stdio.h"

void users(char* argv) {
   
    int i = 0;
    struct User* user;
    while((user = get_user_by_id(i++)) != NULL) {
        printf("name:%s uid:%d gid:%d\n", user->name, user->id, user->gid); 
    }
}


void manUsers(void) {
    setBold(1);
    printf("Usage:\n\t users\n");
    setBold(0);
}
