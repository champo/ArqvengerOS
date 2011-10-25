#include "shell/groups/groups.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "system/accessControlList/groups.h"

void groups(char* argv) {

    int i = 0;
    struct Group* group;
    while((group = get_group_by_id(i++)) != NULL) {
        printf("groupname:%s gid:%d members:%d\n", group->name, group->id, group->num_members);
    }
    
}


void manGroups(void) {
    setBold(1);
    printf("Usage:\n\t groups\n");
    setBold(0);
}
