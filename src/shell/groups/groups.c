#include "shell/groups/groups.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "system/accessControlList/groups.h"

void groups(char* argv) {

    int i = 0, groups = 0;
    int total = get_groups_num();
    printf("total groups ---- %d\n",total);
    struct Group* group;
    while (groups < total ) {
        group = get_group_by_id(i++);

        if (group != NULL) {
            printf("groupname:%s gid:%d members:%d\n", group->name, group->id, group->num_members);
            groups++;
        }
    }
}


void manGroups(void) {
    setBold(1);
    printf("Usage:\n\t groups\n");
    setBold(0);
}
