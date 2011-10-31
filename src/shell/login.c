#include "library/stdio.h"
#include "library/sys.h"
#include "library/string.h"
#include "shell/login.h"
#include "system/accessControlList/users.h"
#include "system/call/ioctl/keyboard.h"
#include "shell/shell.h"

#define cleanbuffer() while(getchar()!='\n')

struct User* askForLogin(void);

void login(char* unused) {

    while(1) {
        
        struct User* user;
        do {
            printf("login:");
        } while ((user = askForLogin()) == NULL);

        setProcessPersona(getpid(), user->id, user->gid[0]);
        
        free_user(user);        
        run(shell, NULL, 1);

        wait();
    }
}


struct User* askForLogin(void) {

    int promptLen = strlen("login");
    char passwd[MAX_PASSWD_LEN];
    char username[MAX_USERNAME_LEN];
    termios passwdTermios = {1 , 0};
    termios userTermios = {1, 1};

    scanf("%s", username);
    cleanbuffer();

    printf("Password:");

    ioctl(0, TCGETS, (void*) &userTermios);
    ioctl(0, TCSETS, (void*) &passwdTermios);

    scanf("%s", passwd);
    cleanbuffer();

    ioctl(0, TCSETS, (void*) &userTermios);

    printf("\n\n");

    struct User* user = get_user_by_name(username);

    if (user != NULL) {
        if (strcmp(passwd, user->passwd) == 0) {
            return user;
        }
    }
    printf("Login incorrect\n");
    return NULL;

}

