#include "shell/renice/renice.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/string.h"
#include "library/sys.h"
#include "type.h"


void command_renice(char* argv) {
    
    char* firstSpace = strchr(argv, ' ');
    char* secondSpace = strchr(firstSpace + 1, ' ');
    char aux[200];
    int pid, new_priority;

    if (firstSpace == NULL || secondSpace == NULL) {
        printf("Usage: renice priority pid\n");
        return;
    }

    strncpy(aux, firstSpace + 1, secondSpace - firstSpace - 1);
    aux[secondSpace - firstSpace - 1] = '\0';

    new_priority = atoi(aux);
    
    pid = atoi(secondSpace + 1);


    if (new_priority < MIN_PRIORITY || new_priority > MAX_PRIORITY || !is_a_number(aux)) {
        printf("renice: invalid priority value\n"); 
        printf("renice: priority values goes from %d to %d\n", MIN_PRIORITY, MAX_PRIORITY);
        printf("renice: please choose another\n");
        return;
    }
    
    if (!is_a_number(secondSpace + 1)) {
        printf("renice: invalid pid value\n");
        printf("renice: pids must be numeric\n");
        printf("renice: please choose another\n");
    }

    new_priority = renice(pid, new_priority);
           
    if (new_priority == INVALID_PRIORITY) {
        printf("renice: invalid pid value\n"); 
        printf("renice: pids must match the pid of an active process\n");
        printf("renice: please choose another\n");
        return;
    }
    
    printf("Successfully changed the priority of process with PID %d to %d\n", pid, new_priority);
}


void manRenice(void) {
    setBold(1);
    printf("Usage:\n\t renice ");
    setBold(0);

    printf("priority pid\n\n");

}
