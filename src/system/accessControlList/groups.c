#include "system/accessControlList/groups.h"
#include "system/accessControlList/users.h"
#include "system/common.h"
#include "system/mm.h"
#include "library/string.h"
#include "library/stdio.h"
#include "library/stdlib.h"

#define MAX_GROUPS  128

static void parseGroupLine(char* line,struct Group* group);
static int updateGroupsFile(struct Group* group, int delete);

//static struct Group* groups[MAX_GROUPS];
//static int groups_num = 0;

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

    //return groups[gid];

    FILE* fp = fopen("/groups", "r");
    disableInterrupts();
    struct Group* group = kalloc(sizeof(struct Group));
    enableInterrupts();

    char line[200];

    do {
        if (fscanf(fp, "%s\n", line) == 0) {
            fclose(fp);
            //kfree(group);
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
    
    //for (int i = 0; i < groups_num; i++) {

        //if (strcmp(groups[i]->name, groupname) == 0) {
            //return groups[i];
        //}
    //}

    //return NULL;

    FILE* fp = fopen("/groups", "r");
    disableInterrupts();
    struct Group* group = kalloc(sizeof(struct Group));
    enableInterrupts();

    char line[200];

    do {
        if (fscanf(fp, "%s\n", line) == 0) {
            fclose(fp);
            //kfree(group);
            return NULL;
        }
        parseGroupLine(line, group);

    } while(strcmp(group->name, groupname) != 0);

    fclose(fp);
    return group;
}

char* get_groupname(int gid) {
    
    //return groups[gid]->name;

    return get_group_by_id(gid)->name;
}

int create_group(char* groupname) {
   
    //if (groups_num == MAX_GROUPS) {
        //return -1;
    //}
    //struct Group* new_group = kalloc(sizeof(struct Group));

    //new_group->id = groups_num;
    //strcpy(new_group->name, groupname);
    //new_group->num_members = 0;

    //groups[groups_num] = new_group;
    //groups_num++;

    //return 0;

    FILE* fp = fopen("/groups", "r+");
    char line[200];
    struct Group* groups[MAX_GROUPS];
    int ids[MAX_GROUPS] = {0};
    int i = 0, id;

    while (fscanf(fp, "%s\n", line) != 0) {
        disableInterrupts();
        groups[i] = kalloc(sizeof(struct Group));
        enableInterrupts();

        parseGroupLine(line, groups[i]);
        ids[groups[i]->id] = 1;
        i++;
    }

    disableInterrupts();
    groups[i] = kalloc(sizeof(struct Group));
    enableInterrupts();
    
    for(int j = 0; j < MAX_GROUPS; j++) {
        if (ids[j] != 1) {
            id = j;
            break;
        }
    }
    
    strcpy(groups[i]->name, groupname);
    groups[i]->id = id;

    updateGroupsFile(groups[i], 0);
    
    return id;
}

int updateGroupsFile(struct Group* group, int delete) {

    FILE* fp = fopen("/groups", "r");
    char line[200][100];
    char aux[100];
    int length, count, found = 0, i = 0;
    
    while (fscanf(fp, "%s\n", line[i++]) != 0);

    fclose(fp);
    fp = fopen("/groups", "w");
    
    for (int j = 0; j < i - 1; j++) {
        
        length = strchr(line[j], ':') - line[j];
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

    fprintf(fp, "%s:x:%d:", group->name, group->id);
                
    while(count != group->num_members) {
        fprintf(fp, "%s",group->members[count]->name);
        if (count != group->num_members - 1) {
            fprintf(fp, ",");
        }
        count++;
    }
    fprintf(fp, "\n");
}

int delete_group(char* name) {
    
    //struct Group* group = get_group_by_name(name);
    //if (group == NULL)  {
        //return -1;
    //}

    //int i = group->id;
    //while (groups[i] != NULL) {
        //groups[i] = groups[i+1];
        //i++;
    //}
    //groups[i] = NULL;
    //kfree(group);
    //groups_num--;

    //return 0;

    struct Group* group = get_group_by_name(name);
    if (group == NULL) {
        return -1;
    }

    updateGroupsFile(group, 1);
    return 0;
}

int add_group_member(int gid, int uid) {

    struct Group* group = get_group_by_id(gid);
    struct User* user = get_user_by_id(uid);
    
    if (group == NULL || user == NULL) {
        return -1;
    }

    //TODO what happens if group is full
    user->gid = gid; 
    group->members[group->num_members] = user;
    group->num_members++;

    updateGroupsFile(group, 0);

    return group->members;
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


