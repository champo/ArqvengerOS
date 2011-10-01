#include "shell/top/top.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "mcurses/mcurses.h"
#include "system/process/pinfo.h"
#include "library/call.h"
#include "system/call/codes.h"

void top(char* arv) {

    printf("PID\tPPID\tUSER\t%%CPU\tPRIO\tSTATE\tSTART DATE\n");
    struct ProcessInfo data[20];
    int pcount = system_call(_SYS_PINFO, data, 20,0);
    time_t rawTime = time(NULL);
    struct tm* timeInfo = localtime(&rawTime);
    char *date = asctime(timeInfo);

    for (int i = 0; i < pcount; i++) {
        printf("%d\t%d\t",data[i].pid, data[i].ppid);
        printf("%s\t%d\t", "acrespo", 20);
        printf("%d\t%s\t",data[i].priority, (data[i].state)?"alive":"zombie");
        printf("%s",date);
        printf("\n");
    }
}



void manTop(void) {
    setBold(1);
    printf("Usage:\n\t top\n");
    setBold(0);
}
