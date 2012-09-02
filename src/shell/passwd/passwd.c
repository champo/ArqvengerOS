#include "shell/passwd/passwd.h"
#include "shell/utils.h"
#include "system/accessControlList/users.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"
#include "library/string.h"
#include "drivers/videoControl.h"
#include "library/sys.h"


void passwd(char* argv) {

    UNUSED(argv);

    char old_passwd[MAX_PASSWD_LEN];
    char new_passwd1[MAX_PASSWD_LEN];
    char new_passwd2[MAX_PASSWD_LEN];
    char username[200];
    int uid, gid;
    struct User* me;

    getProcessPersona(getpid(), &uid, &gid);
    me = get_user_by_id(uid);
    strcpy(username, me->name);
    free_user(me);

    printf("Changing password for %s\n", username);

    askForPasswd("(current) password:", old_passwd);

    if (strcmp(old_passwd, me->passwd) != 0) {
        printf("passwd: Authentication failure\n");
        printf("passwd: password unchanged\n");
        return;
    }

    askForPasswd("Enter new password:", new_passwd1);
    askForPasswd("Retype new password:", new_passwd2);

    if (strcmp(new_passwd1, new_passwd2) != 0) {
        printf("Sorry, passwords do not match\n");
        printf("passwd: Authentication information cannot be recovered\n");
        printf("passwd: password unchanged\n");
        return;
    }

    change_passwd(uid, old_passwd, new_passwd1);

    printf("passwd: password updated succesfully\n");
}



void manPasswd(void) {
    setBold(1);
    printf("Usage:\n\t passwd ");
    setBold(0);

    printf("[ LOGIN ]\n");

    setBold(1);
    printf("DESCRIPTION\n");
    setBold(0);

    printf("The");

    setBold(1);
    printf("passwd");
    setBold(0);

    printf("command changes passwords for user accounts. A normal \
            user may only change the password for his/her own account, while \
            the superuser may change the password for any account.\n");

    setBold(1);
    printf("passwd");
    setBold(0);

    printf("also changes the account or associated password validity period.\n");

}

