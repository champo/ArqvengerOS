#ifndef __EXT2__
#define __EXT2__

struct ext2;

struct ext2* ext2_load(unsigned long long startSector);

#endif
