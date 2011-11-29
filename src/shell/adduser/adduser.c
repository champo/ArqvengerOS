#include "shell/adduser/adduser.h"
#include "shell/utils.h"
#include "system/accessControlList/users.h"
#include "system/accessControlList/groups.h"
#include "library/string.h"
#include "library/stdio.h"
#include "drivers/videoControl.h"
#include "mcurses/mcurses.h"


void adduser(char* argv) {
    char username[MAX_USERNAME_LEN];
    char groupname[MAX_GROUPNAME_LEN];
    char passwd[MAX_PASSWD_LEN];
    char proceed[LINE_WIDTH];
    int uid;
    int first = 1;
    struct User* user;
    struct Group* group;


    printf("\n");

    do {
        if (!first) {
            printf("-Username invalid or already taken\n");
            printf("-Please choose another\n");
        }

        askForInput("Login name for new user []:", username);
        first = 0;

    } while(strcmp(username,"") == 0 ||  (user = get_user_by_name(username)) != NULL);

    free_user(user);
    printf("\n");

    first = 1;
    do {
        if (!first) {
            printf("-Group '%s' does not exists\n", groupname);
            printf("-Please choose another\n");
        }

        askForInput("Initial group [ users ]:", groupname);

        if (strcmp(groupname, "") == 0) {
            strcpy(groupname, "users");
        }

        first = 0;
    } while ((group = get_group_by_name(groupname)) == NULL);


    printf("\nNew account will be created as follows:\n");
    printf("------------------------------------------\n");
    printf("Login name: %s\n", username);
    printf("UID: [ Next available ]\n");
    printf("Initial group: %s\n", groupname);
    printf("\n");

    do {
        askForInput("This is it... if you want to bail out, type quit. Otherwise, press\n"
                "ENTER to go ahead and make the account.\n", proceed);

    } while (strcmp(proceed, "quit") != 0 && strcmp(proceed, "") != 0);

    if (strcmp(proceed, "quit") == 0) {
        exit();
    }

    printf("\n\nCreating new account...\n");

    askForPasswd("Password:", passwd);

    uid = create_user(username, passwd, groupname);

    if (uid == -1) {
        printf("adduser: unable to create new user\n");
        printf("adduser: out of space\n");
        printf("adduser: try deleting another user and trying again\n");
        return;
    }

    if (add_group_member(group->id, uid) == -1) {
        printf("adduser: could not add user to the group\n");
        printf("adduser: it is very likely that the maximum capacity has been reached\n");
        printf("adduser: try deleting other users to make room\n");

        delete_user(username);
        free_group(group);
        return;
    }

    free_group(group);
    printf("Successfully created user account.\n");
}


void manAdduser(void) {
    setBold(1);
    printf("Usage:\n\t adduser\n");
    setBold(0);
}
