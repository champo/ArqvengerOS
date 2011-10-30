#include "shell/passwd/passwd.h"
#include "system/accessControlList/users.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"
#include "library/string.h"
#include "drivers/videoControl.h"
#include "system/call/ioctl/keyboard.h"
#include "library/sys.h"

#define cleanbuffer() while(getchar()!='\n')

void passwd(char* argv) {

    char old_passwd[MAX_PASSWD_LEN];
    char new_passwd1[MAX_PASSWD_LEN];
    char new_passwd2[MAX_PASSWD_LEN];
    int uid, gid;
    struct User* me;
    termios oldTermios;
    termios passwdTermios = {1 , 0};

    getProcessPersona(getpid(), &uid, &gid);
    me = get_user_by_id(uid);

    printf("Changing password for %s\n", me->name);

    printf("(current) password:");

    ioctl(0, TCGETS, (void*) &oldTermios);
    ioctl(0, TCSETS, (void*) &passwdTermios);
    
    scanf("%s", old_passwd);
    cleanbuffer();

    ioctl(0, TCSETS, (void*) &oldTermios);
    printf("\n");


    //printf("I AM %s uid: %d gid: %d\n",me->name, me->id, me->gid[0]);

    if (strcmp(old_passwd, me->passwd) != 0) {
        printf("passwd: Authentication failure\n");
        printf("passwd: password unchanged\n");
        return;
    }
    
    printf("Enter new password:");
    
    ioctl(0, TCGETS, (void*) &oldTermios);
    ioctl(0, TCSETS, (void*) &passwdTermios);
    
    scanf("%s", new_passwd1);
    cleanbuffer();
    
    ioctl(0, TCSETS, (void*) &oldTermios);
    printf("\n");

    printf("Retype new password:");

    ioctl(0, TCGETS, (void*) &oldTermios);
    ioctl(0, TCSETS, (void*) &passwdTermios);
    
    scanf("%s", new_passwd2);
    cleanbuffer();
    
    ioctl(0, TCSETS, (void*) &oldTermios);
    printf("\n");

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

    //TOO MUCH???
    
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

