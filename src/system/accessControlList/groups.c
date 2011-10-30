#include "system/accessControlList/groups.h"
#include "system/accessControlList/users.h"
#include "system/common.h"
#include "system/malloc/malloc.h"
#include "library/string.h"
#include "library/stdio.h"
#include "library/stdlib.h"


static void parseGroupLine(char* line,struct Group* group);
static int updateGroupsFile(struct Group* group, int delete);
static void writeGroupLine(FILE* fp, struct Group* group);

int get_groups_num(void) {

    FILE* fp = fopen("/groups", "r");
    int groups = 0;
    char c;

    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            groups++;
        }
    }

    fclose(fp);
    return groups;

}

struct Group* get_group_by_id(int gid) {

    FILE* fp = fopen("/groups", "r");
    struct Group* group = malloc(sizeof(struct Group));
    char line[200];

    do {
        if (fscanf(fp, "%s\n", line) == 0) {
            fclose(fp);
            free(group);
            return NULL;
        }

        parseGroupLine(line, group);

    } while(group->id != gid);

    fclose(fp);
    return group;
}

void parseGroupLine(char* line, struct Group* group) {

    char* nextColumn = strchr(line, ':');
    char* nextComma;
    char* buf = line;
    char aux[200];
    int i;

    memset(group, 0, sizeof(struct Group));

    strncpy(group->name, buf, nextColumn - buf);
    group->name[nextColumn - buf] = '\0';
    buf = nextColumn + 1;

    nextColumn = strchr(buf, ':');
    buf = nextColumn + 1;

    nextColumn = strchr(buf, ':');
    strncpy(aux, buf, nextColumn - buf);
    aux[nextColumn - buf] = '\0';
    buf = nextColumn + 1;

    group->id = atoi(aux);

    i = 0;
    while((nextComma = strchr(buf, ',')) != NULL) {
        strncpy(aux, buf, nextComma - buf);
        aux[nextComma - buf] = '\0';

        group->members[i] = get_user_by_name(aux);
        group->num_members = i+1;
        buf = nextComma + 1;
        i++;
    }

    if (strcmp(buf, "") != 0) {
        //if buf is not empty there's a last member
        strcpy(aux, buf);
        group->members[i] = get_user_by_name(aux);
        group->num_members = i+1;
    }
}


struct Group* get_group_by_name(char* groupname) {

    FILE* fp = fopen("/groups", "r");
    struct Group* group = malloc(sizeof(struct Group));

    char line[200];

    do {
        if (fscanf(fp, "%s\n", line) == 0) {
            fclose(fp);
            free(group);
            return NULL;
        }

        parseGroupLine(line, group);

    } while(strcmp(group->name, groupname) != 0);

    fclose(fp);
    return group;
}

char* get_groupname(int gid) {

    return get_group_by_id(gid)->name;
}

int create_group(char* groupname) {

    FILE* fp = fopen("/groups", "r+");
    char line[200];
    struct Group* groups[MAX_GROUPS];
    int ids[MAX_GROUPS] = {0};
    int i = 0, id;

    while (fscanf(fp, "%s\n", line) != 0 && i < MAX_GROUPS) {
        groups[i] = malloc(sizeof(struct Group));

        parseGroupLine(line, groups[i]);
        ids[groups[i]->id] = 1;
        i++;
    }
    fclose(fp);

    if (i >= MAX_GROUPS) {

        for (int j = 0; j < i; j++) {
            free(groups[j]);
        }

        return -1;
    }

    groups[i] = malloc(sizeof(struct Group));
    i++;

    for(int j = 0; j < MAX_GROUPS; j++) {
        if (ids[j] != 1) {
            id = j;
            break;
        }
    }

    strcpy(groups[i - 1]->name, groupname);
    groups[i - 1]->id = id;
    groups[i - 1]->num_members = 0;

    updateGroupsFile(groups[i - 1], 0);

    for (int j = 0; j < i; j++) {
        free(groups[j]);
    }

    return id;
}

int updateGroupsFile(struct Group* group, int delete) {

    FILE* fp = fopen("/groups", "r");
    char line[MAX_GROUPS][200];
    char aux[MAX_GROUPNAME_LEN + 1];
    int found = 0, i = 0;

    while (fscanf(fp, "%s\n", line[i++]) != 0);

    fclose(fp);
    fp = fopen("/groups", "w");

    for (int j = 0; j < i - 1; j++) {

        size_t length = strchr(line[j], ':') - line[j];
        strncpy(aux, line[j], length);
        aux[length] = '\0';

        if (strcmp(aux, group->name) == 0) {

            if (!delete) {
                writeGroupLine(fp, group);
                found = 1;
            }

        } else {
            fprintf(fp, "%s\n" ,line[j]);
        }
    }

    if (!found && !delete) {
        writeGroupLine(fp, group);
    }

    fclose(fp);
    return 0;
}

void writeGroupLine(FILE* fp, struct Group* group) {
    int count = 0;
    int i = 0;

    fprintf(fp, "%s:x:%d:", group->name, group->id);

    while (count < group->num_members && i < MAX_GROUP_MEMBERS) {
        if (group->members[i] != NULL) {

            fprintf(fp, "%s",group->members[i]->name);
            if (count != group->num_members - 1) {
                fprintf(fp, ",");
            }
            count++;
        }
        i++;
    }
    fprintf(fp, "\n");
}

int delete_group(char* name) {

    struct Group* group = get_group_by_name(name);
    if (group == NULL || group->num_members > 0) {
        return -1;
    }

    updateGroupsFile(group, 1);

    free(group);
    return 0;
}

int add_group_member(int gid, int uid) {

    struct Group* group = get_group_by_id(gid);
    struct User* user = get_user_by_id(uid);

    if (group == NULL || user == NULL) {
        return -1;
    }

    //what happens if group is full
    if (group->num_members >= MAX_GROUP_MEMBERS) {
        return -1;
    }

    user->gid[0] = gid;
    group->members[group->num_members] = user;
    group->num_members++;

    updateGroupsFile(group, 0);

    return group->num_members;
}

int delete_group_member(int gid, int uid) {

    struct Group* group = get_group_by_id(gid);
    struct User* user = get_user_by_id(uid);

    if (group == NULL || user == NULL) {
        return -1;
    }

    for(int i = 0; i < group->num_members; i++) {
        if (group->members[i]->id == user->id) {
            group->members[i] = NULL;
            group->num_members--;

            updateGroupsFile(group, 0);

            return group->num_members;

        }
    }

    return -1;
}


