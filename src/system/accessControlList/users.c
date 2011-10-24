#include "system/accessControlList/users.h"
#include "system/mm.h"
#include "library/string.h"

#define INVALID_PASSWD  1
#define PASSWD_CHANGED  0
#define MAX_USERS       128


static struct User* registered_users[MAX_USERS];
static int registered_users_num = 0;

struct User* get_user_by_id(int id) {

    return registered_users[id];
}

struct User* get_user_by_name(char* name) {
    
    for (int i = 0; i < registered_users_num; i++) {
        
        if (strcmp(registered_users[i]->name, name) == 0) {
            return registered_users[i];
        }
    }
    return NULL;
}


char* get_username(int id) {
    
    return registered_users[id]->name;
}

int change_passwd(int uid, char* old_passwd, char* new_passwd) {
    
    struct User* user = registered_users[uid];

    if (strcmp(user->passwd, old_passwd) != 0) {
        
        strcmp(user->passwd, new_passwd);
        return PASSWD_CHANGED;
    } else {

        return INVALID_PASSWD;
    }
}

int create_user(char* name, char* passwd) {

    if (registered_users_num == MAX_USERS) {
        return -1; 
    }

    struct User* new_user = kalloc(sizeof(struct User));

    new_user->id = registered_users_num;
    strcpy(new_user->name, name); 
    strcpy(new_user->passwd, passwd); 
   
    registered_users[registered_users_num] = new_user;
    registered_users_num++;

    return new_user->id;
}
