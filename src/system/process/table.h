#ifndef __SYSTEM_PROCESS_TABLE__
#define __SYSTEM_PROCESS_TABLE__

#include "system/process/process.h"

struct Process* process_table_new(EntryPoint entryPoint, char* args, struct Process* parent);

void process_table_exit(struct Process* process);

pid_t process_table_wait(struct Process* process);

struct Process* process_table_get(pid_t pid);

void process_table_kill(struct Process* process);

#endif
