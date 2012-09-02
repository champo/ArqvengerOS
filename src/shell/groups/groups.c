#include "shell/groups/groups.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "system/accessControlList/groups.h"

void groups(char* argv) {

    UNUSED(argv);

    int i = 0, processed = 0;
    int total = get_groups_num();
    struct Group* group;

    printf("total groups ---- %d\n",total);

    while (processed < total ) {
        group = get_group_by_id(i++);

        if (group != NULL) {

            printf("groupname:%s gid:%d members:%d\n", group->name, group->id, group->num_members);
            processed++;

            free_group(group);
        }
    }
}


void manGroups(void) {
    setBold(1);
    printf("Usage:\n\t groups\n");
    setBold(0);
}
