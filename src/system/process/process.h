#ifndef __SYSTEM_PROCESS_PROCESS__
#define __SYSTEM_PROCESS_PROCESS__

#include "system/process/table.h"

void createProcess(struct Process* process, void* entrypoint);

void destroyProcess(struct Process* process);

#endif
