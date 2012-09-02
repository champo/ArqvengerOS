#include "library/stdio.h"
#include "library/sys.h"
#include "library/string.h"
#include "shell/login.h"
#include "system/accessControlList/users.h"
#include "system/call/ioctl/keyboard.h"
#include "shell/shell.h"
#include "shell/utils.h"


struct User* askForLogin(void);

void login(char* unused) {

    UNUSED(unused);

    while(1) {

        struct User* user;
        while ((user = askForLogin()) == NULL);

        setProcessPersona(getpid(), user->id, user->gid[0]);

        free_user(user);
        run(shell, "shell", 1);

        wait();
    }
}


struct User* askForLogin(void) {

    char passwd[MAX_PASSWD_LEN];
    char username[MAX_USERNAME_LEN];

    askForInput("Login:", username);

    askForPasswd("Password:", passwd);

    printf("\n");

    struct User* user = get_user_by_name(username);

    if (user != NULL) {
        if (strcmp(passwd, user->passwd) == 0) {
            return user;
        }
    }

    printf("Login incorrect\n");

    return NULL;

}

