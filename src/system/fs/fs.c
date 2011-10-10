#include "drivers/ext2/ext2.h"
#include "system/fs/fs.h"

struct Filesystem* file_system_load(unsigned long long startSector) {
    return ext2_load(startSector);
}
