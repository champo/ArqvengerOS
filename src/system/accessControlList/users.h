#ifndef __SYSTEM_ACCESSCONTROLLIST_USERS__
#define __SYSTEM_ACCESSCONTROLLIST_USERS__

#define MAX_USERNAME_LEN    32
#define MAX_PASSWD_LEN      32

struct User {
    int id;
    int gid;
    char name[MAX_USERNAME_LEN];
    char passwd[MAX_PASSWD_LEN];
};

struct User* get_user_by_id(int uid);

struct User* get_user_by_name(char* name);

char* get_username(int uid);

int change_passwd(int uid, char* old_passwd, char* new_passwd);

int create_user(char* name, char* passwd);

int delete_user(char* name);

#endif


