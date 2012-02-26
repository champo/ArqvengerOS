#ifndef __SYSTEM_ACCESSCONTROLLIST_GROUPS__
#define __SYSTEM_ACCESSCONTROLLIST_GROUPS__

#define MAX_GROUPNAME_LEN   32
#define MAX_GROUP_MEMBERS   128
#define MAX_GROUPS  1

struct Group {
    char name[MAX_GROUPNAME_LEN];
    int id;
    struct User* members[MAX_GROUP_MEMBERS];
    int num_members;
};

int get_groups_num(void);

struct Group* get_group_by_id(int gid);

struct Group* get_group_by_name(char* groupname);

char* get_groupname(int gid);

int create_group(char* groupname);

int delete_group(char* groupname);

int add_group_member(int gid, int uid);

int delete_group_member(int gid, int uid);

void free_group(struct Group* group);

#endif
