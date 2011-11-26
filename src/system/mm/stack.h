#ifndef __SYSTEM_MM_STACK__
#define __SYSTEM_MM_STACK__

#define MAX_PAGES_IN_STACK  256

void page_fault_handler(int errCode);

#endif

