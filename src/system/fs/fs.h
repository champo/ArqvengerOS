#ifndef __FILE_SYSTEM__
#define __FILE_SYSTEM__

#include "drivers/ext2/ext2.h"

#define Filesystem ext2
//typedef struct ext2 struct Filesystem;

struct Filesystem* file_system_load(unsigned long long startSector); 

#endif
