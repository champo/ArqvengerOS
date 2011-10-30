#ifndef __SYSTEM_ACCESSCONTROLLIST_USERS__
#define __SYSTEM_ACCESSCONTROLLIST_USERS__

#include "system/accessControlList/groups.h"

#define MAX_USERS       128
#define MAX_USERNAME_LEN    32
#define MAX_PASSWD_LEN      32

struct User {
    int id;
    int gid[MAX_GROUPS];
    char name[MAX_USERNAME_LEN];
    char passwd[MAX_PASSWD_LEN];
};

void users_init(void);

int get_users_num(void);

struct User* get_user_by_id(int uid);

struct User* get_user_by_name(char* name);

char* get_username(int uid);

int change_passwd(int uid, char* old_passwd, char* new_passwd);

int create_user(char* name, char* passwd, char* groupname);

int delete_user(char* name);

#endif


