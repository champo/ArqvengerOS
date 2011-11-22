#include "shell/users/users.h"
#include "mcurses/mcurses.h"
#include "system/accessControlList/users.h"
#include "library/stdio.h"

void users(char* argv) {
   
    int i = 0, users = 0;
    int total = get_users_num();
    printf("total users ---- %d\n",total);
    struct User* user;
    while(users < total) {
        user = get_user_by_id(i++);
        
        if (user != NULL) {
            printf("name:%s uid:%d gid:%d\n", user->name, user->id, user->gid[0]);
            users++;
            free_user(user);
        } 
    }
}


void manUsers(void) {
    setBold(1);
    printf("Usage:\n\t users\n");
    setBold(0);
}
