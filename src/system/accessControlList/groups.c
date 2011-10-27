#include "system/accessControlList/groups.h"
#include "system/accessControlList/users.h"
#include "system/mm.h"
#include "library/string.h"
#include "library/stdio.h"

#define MAX_GROUPS  128


static struct Group* groups[MAX_GROUPS];
static int groups_num = 0;

struct Group* get_group_by_id(int gid) {

    return groups[gid];
}

struct Group* get_group_by_name(char* groupname) {
    
    for (int i = 0; i < groups_num; i++) {

        if (strcmp(groups[i]->name, groupname) == 0) {
            return groups[i];
        }
    }

    return NULL;
}

char* get_groupname(int gid) {
    
    return groups[gid]->name;
}

int create_group(char* groupname) {
   
    if (groups_num == MAX_GROUPS) {
        return -1;
    }
    struct Group* new_group = kalloc(sizeof(struct Group));

    new_group->id = groups_num;
    strcpy(new_group->name, groupname);
    new_group->num_members = 0;

    groups[groups_num] = new_group;
    groups_num++;

    return 0;
}


int delete_group(char* name) {
    
    struct Group* group = get_group_by_name(name);
    if (group == NULL)  {
        return -1;
    }

    int i = group->id;
    while (groups[i] != NULL) {
        groups[i] = groups[i+1];
        i++;
    }
    groups[i] = NULL;
    kfree(group);
    groups_num--;

    return 0;
}

int add_group_member(int gid, int uid) {

    struct Group* group = groups[gid];
    struct User* user = get_user_by_id(uid);

    //TODO what happens if group is full
    user->gid = gid;
    group->members[group->num_members] = user; 
    
    group->num_members++;
    return group->num_members;
}

int delete_group_member(int gid, int uid) {

    struct Group* group = groups[gid];
    struct User* user = get_user_by_id(uid);

    if (group == NULL || user == NULL) {
        return -1;
    }
    
    for (int i = 0; i < group->num_members; i++) {
        if (group->members[i]->id == user->id) {
            group->members[i] = NULL;
            group->num_members--;
            return group->num_members;
        }
    }    

    return -1;
}


