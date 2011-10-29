#include "shell/adduser/adduser.h"
#include "system/accessControlList/users.h"
#include "system/accessControlList/groups.h"
#include "system/call/ioctl/keyboard.h"
#include "library/string.h"
#include "library/stdio.h"
#include "drivers/videoControl.h"
#include "mcurses/mcurses.h"

#define cleanbuffer() while(getchar()!='\n')


void adduser(char* argv) {
    char username[MAX_USERNAME_LEN];
    char groupname[MAX_GROUPNAME_LEN];
    char passwd[MAX_PASSWD_LEN];
    char proceed[LINE_WIDTH];
    int uid;
    int first = 1;
    termios oldTermios;
    termios passwdTermios = {1 , 0};

    printf("\n");

    do {
        printf("Login name for new user []:");
        scanf("%s",username);
        cleanbuffer();
    } while(strcmp(username,"") == 0);

    printf("\n");
    
    do {
        if (!first) {
            printf("-Group '%s' does not exists\n", groupname);
            printf("-Please choose another\n");
        }
        printf("Initial group [ users ]:");
        scanf("%s", groupname);
        cleanbuffer();
    
        if (strcmp(groupname, "") == 0) {
            strcpy(groupname, "users");
        }

        first = 0;
    } while (get_group_by_name(groupname) == NULL);
    
    printf("\nNew account will be created as follows:\n");
    printf("------------------------------------------\n");
    printf("Login name: %s\n", username);
    printf("UID: [ Next available ]\n");
    printf("Initial group: %s\n", groupname);
    
    printf("\n");
    do {
        printf("This is it... if you want to bail out, type quit. Otherwise, press\n");
        printf("ENTER to go ahead and make the account.\n");
        scanf("%s", proceed);
        cleanbuffer();

    } while (strcmp(proceed, "quit") != 0 && strcmp(proceed, "") != 0);

    printf("\n\nCreating new account...\n");
    
    printf("Password:");
    
    ioctl(0, TCGETS, (void*) &oldTermios);
    ioctl(0, TCSETS, (void*) &passwdTermios);
    
    scanf("%s", passwd);
    cleanbuffer();

    ioctl(0, TCSETS, (void*) &oldTermios);
    printf("\n");

    uid = create_user(username, passwd);
    
    add_group_member(uid,1);
}


void manAdduser(void) {
    setBold(1);
    printf("Usage:\n\t adduser\n");
    setBold(0);
}
