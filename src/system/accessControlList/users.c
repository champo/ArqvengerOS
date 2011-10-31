#include "system/accessControlList/users.h"
#include "system/common.h"
#include "system/malloc/malloc.h"
#include "library/string.h"
#include "library/stdio.h"
#include "library/stdlib.h"

#define INVALID_PASSWD  1
#define PASSWD_CHANGED  0

static void parseUserLine(char* str, struct User* user, char* def_group);
static int updateUsersFile(struct User* user, int delete);
static void writeUserLine(FILE* fp, struct User* user);

void users_init(void) {

}

int get_users_num(void) {

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

    FILE* fp = fopen("/users", "r");
    struct User* user = malloc(sizeof(struct User));
    char str[200];
    char def_group[100];

    do {
        if (fscanf(fp, "%s\n", str) == 0) {
            fclose(fp);
            free(user);
            return NULL;
        }

        parseUserLine(str, user, def_group);

    } while(user->id != id);

    fclose(fp);
    return user;
}

void parseUserLine(char* str, struct User* user, char* def_group) {

    char* nextColumn = strchr(str, ':');
    char* buf = str;
    char aux[200];

    strncpy(user->name, buf, nextColumn - buf);
    user->name[nextColumn - buf] = '\0';
    buf = nextColumn + 1;

    nextColumn = strchr(buf, ':');
    buf = nextColumn + 1;

    nextColumn = strchr(buf, ':');
    strncpy(aux, buf, nextColumn - buf);
    aux[nextColumn - buf] = '\0';

    buf = nextColumn + 1;

    user->id = atoi(aux);

    nextColumn = strchr(buf, ':');
    strncpy(aux, buf, nextColumn - buf);
    aux[nextColumn - buf] = '\0';

    buf = nextColumn + 1;

    user->gid[0] = atoi(aux);

    nextColumn = strchr(buf, ':');
    strncpy(aux, buf, nextColumn - buf);
    aux[nextColumn - buf] = '\0';

    strcpy(user->passwd, aux);

    buf = nextColumn + 1;

    strcpy(def_group, aux);
}

struct User* get_user_by_name(char* name) {

    FILE* fp = fopen("/users", "r");
    struct User* user = malloc(sizeof(struct User));

    char str[200];
    char def_group[100];
    do {
        if (fscanf(fp, "%s\n", str) == 0) {
            fclose(fp);
            free(user);
            return NULL;
        }

        parseUserLine(str, user, def_group);

    } while(strcmp(user->name, name) != 0);

    fclose(fp);
    return user;
}

char* get_username(int id) {
    struct User* user = get_user_by_id(id);
    char* name = user->name;
    free_user(user);
    return user->name;
}

int change_passwd(int uid, char* old_passwd, char* new_passwd) {

    struct User* user = get_user_by_id(uid);

    if (strcmp(user->passwd, old_passwd) == 0) {
        strcpy(user->passwd, new_passwd);

        updateUsersFile(user, 0);
        
        free_user(user);
        return PASSWD_CHANGED;
    } else {
        free_user(user);
        return INVALID_PASSWD;
    }
}

int updateUsersFile(struct User* user, int delete) {
    FILE* fp = fopen("/users", "r");
    char line[200][100];
    char aux[100];
    int length, found = 0, i = 0;

    while(fscanf(fp, "%s\n", line[i++]) != 0);

    fclose(fp);
    fp = fopen("/users", "w");

    for (int j = 0; j < i - 1; j++) {

        length = strchr(line[j], ':') - line[j];
        strncpy(aux, line[j], length);
        aux[length] = '\0';

        if (strcmp(user->name, aux) == 0) {
            if (!delete) {
                writeUserLine(fp, user);
                found = 1;
            }
        } else {
            fprintf(fp, "%s\n", line[j]);
        }
    }

    if (!found && !delete) {
        writeUserLine(fp, user);
    }

    fclose(fp);
    return 0;
}

void writeUserLine(FILE* fp, struct User* user) {

    fprintf(fp, "%s:x:%d:%d:", user->name, user->id, user->gid[0]);
    fprintf(fp, "%s:%s\n", user->passwd, get_groupname(user->gid[0]));
}

int create_user(char* name, char* passwd, char* groupname) {

    FILE* fp = fopen("/users", "r+");
    char line[200];
    char buf[100];
    struct User* users[MAX_USERS];
    struct Group* group;
    int ids[MAX_USERS] = {0};
    int i = 0, id;

    while(fscanf(fp, "%s\n", line) != 0 && i < MAX_USERS) {
        users[i] = malloc(sizeof(struct User));

        parseUserLine(line, users[i], buf);
        ids[users[i]->id] = 1;
        i++;
    }
    fclose(fp);

    if (i >= MAX_USERS) {

        for (int j = 0; j < i; j++) {
            free(users[j]);
        }

        return -1;
    }

    users[i] = malloc(sizeof(struct User));
    i++;

    for (int j = 0; j < MAX_USERS; j++) {
        if (ids[j] != 1) {
            id = j;
            break;
        }
    }

    strcpy(users[i - 1]->name, name);
    strcpy(users[i - 1]->passwd, passwd);

    group = get_group_by_name(groupname);
    users[i - 1]->gid[0] = group->id;
    users[i - 1]->id = id;

    updateUsersFile(users[i - 1], 0);

    for (int j = 0; j < i; j++) {
        free(users[j]);
    }

    return id;
}

int delete_user(char* name) {

    struct User* user = get_user_by_name(name);
    if (user == NULL) {
        return -1;
    }

    updateUsersFile(user, 1);

    free(user);
    return 0;
}

void free_user(struct User* user) {
    free(user);
}

