#include "system/accessControlList/users.h"
#include "system/mm.h"
#include "system/common.h"
#include "library/string.h"
#include "library/stdio.h"
#include "library/stdlib.h"

#define INVALID_PASSWD  1
#define PASSWD_CHANGED  0
#define MAX_USERS       128

void parseUserLine(char* str, struct User* user, char* def_group);
int updateUserFile(struct User* user); 

static struct User* registered_users[MAX_USERS];
static int registered_users_num = 0;

void users_init(void) {

}    



int get_users_num(void) {

    //return registered_users_num;

    FILE* fp = fopen("/users", "r");
    int users = 0;
    char c;

    while((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            users++;
        }
    }
    fclose(fp);
    return users;
}

struct User* get_user_by_id(int id) {
   
    //return registered_users_[id];

    FILE* fp = fopen("/users", "r");
    disableInterrupts();
    struct User* user = kalloc(sizeof(struct User));
    enableInterrupts();
    
    char str[200];
    char def_group[100];
    int first = 1;
    char c;
    do {
        //printf("ARCHIVO:------\n");
        //while ((c = fgetc(fp)) != EOF) {
            //printf("%c",c);
        //}
        //printf("\n");
        int as = fscanf(fp, "%s", str);
        printf("str %s\n",str);
        printf("scanf devuelve %d\n",as);
        parseUserLine(str, user, def_group);
        printf("uid %d\n", user->id);       
        printf("gid %d\n", user->gid);       
        printf("name %s\n", user->name);       
        printf("pass %s\n", user->passwd);
        printf("group %s\n", def_group);
        //if(!first) {
        //while(1); 
        //}
        first = 0;
    } while(user->id != id);

    fclose(fp);
    return user;
}

void parseUserLine(char* str, struct User* user, char* def_group) {

        char* nextColumn = strchr(str, ':');
        char aux[200];
        
        strncpy(user->name, str, nextColumn - str);
        user->name[nextColumn - str] = '\0';
        
        strcpy(str, nextColumn + 1);
        //printf("%s\n",str);

        nextColumn = strchr(str, ':');
        strcpy(str, nextColumn + 1);
        //printf("%s\n",str);
    
        nextColumn = strchr(str, ':');
        strncpy(aux, str, nextColumn - str);
        aux[nextColumn - str] = '\0';

        strcpy(str, nextColumn + 1);
       
        user->id = atoi(aux); 
        //printf("uid %d\n",atoi(aux));
    
        nextColumn = strchr(str, ':');
        strncpy(aux, str, nextColumn - str);
        aux[nextColumn - str] = '\0';

        strcpy(str, nextColumn + 1);
        
        user->gid = atoi(aux);
        //printf("gid %d\n", atoi(aux));
    
        nextColumn = strchr(str, ':');
        strncpy(aux, str, nextColumn - str);
        aux[nextColumn - str] = '\0';

        strcpy(user->passwd, aux);
        //printf("%s\n",aux);

        strcpy(str, nextColumn + 1);

        strcpy(def_group, aux);
}

struct User* get_user_by_name(char* name) {
    
    //for (int i = 0; i < registered_users_num; i++) {
        
        //if (strcmp(registered_users[i]->name, name) == 0) {
            //return registered_users[i];
        //}
    //}
    //return NULL;
    
    
    FILE* fp = fopen("/users", "r");
    disableInterrupts();
    struct User* user = kalloc(sizeof(struct User));
    enableInterrupts();
    
    char str[200];
    char def_group[100];
    do {
        fscanf(fp, "%s\n", str);
        parseUserLine(str, user, def_group);
        //printf("uid %d\n", user->id);       
        //printf("gid %d\n", user->gid);       
        //printf("name %s\n", user->name);       
        //printf("pass %s\n", user->passwd);
        //printf("group %s\n", def_group);

    } while(strcmp(user->name, name) != 0);

    fclose(fp);
    return user;
}

char* get_username(int id) {
    
    //return registered_users[id]->name;
    return get_user_by_id(id)->name;
}

int change_passwd(int uid, char* old_passwd, char* new_passwd) {
    
    struct User* user = get_user_by_id(uid);
     
    if (strcmp(user->passwd, old_passwd) == 0) {
        strcpy(user->passwd, new_passwd);
        updateUserFile(user);    
        return PASSWD_CHANGED;
    } else {
        return INVALID_PASSWD;
    }
}

int updateUserFile(struct User* user) {
    FILE* fp = fopen("/users", "r+");
    char line[200][100];
    int ret, i = 0;

    do {
        ret = fscanf(fp, "%s\n", line[i++]);
        printf("line %s\n", line[i-1]);
        printf("%d\n",ret);
        //printf("esto es lo q hay en elarchuivo %c as\n",fgetc(fp));
    } while(ret != EOF && ret != 0);
    
    for (int j= 0; j < i; j++)
        printf("%s\n",line[j]); 

    fclose(fp);
    fp = fopen("/users", "w");
    

    for (int j = 0;j < i; j++) {
        if (strncmp(user->name, line[j], strlen(user->name)) == 0) {
            fprintf(fp, "%s:x:%d:%d:%s:%s\n", user->name, user->id, user->gid, user->passwd, "users");
        } else {
            fprintf(fp, line[j]);
        }
    }
    
    fclose(fp);
    return 0;
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

int delete_user(char* name) {
    
    //TODO make this right!
    struct User* user = get_user_by_name(name);
    if (user == NULL) {
        return -1;
    }

    registered_users[user->id] = NULL;
    registered_users_num--;
    
    delete_group_member(user->gid, user->id);
    kfree(user);

    return 0;
}
