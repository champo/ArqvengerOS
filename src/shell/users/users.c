#include "shell/users/users.h"
#include "mcurses/mcurses.h"
#include "system/accessControlList/users.h"
#include "library/stdio.h"

void users(char* argv) {

    UNUSED(argv);

    int i = 0, found = 0;
    int total = get_users_num();
    struct User* user;

    printf("total users ---- %d\n", total);

    while(found < total) {
        user = get_user_by_id(i++);

        if (user != NULL) {
            printf("name:%s uid:%d gid:%d\n", user->name, user->id, user->gid[0]);
            found++;
            free_user(user);
        }
    }
}


void manUsers(void) {
    setBold(1);
    printf("Usage:\n\t users\n");
    setBold(0);
}
