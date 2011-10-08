#ifndef __DRIVER_ATA__
#define __DRIVER_ATA__

#include "multiboot.h"

#define SECTOR_SIZE 512

void ata_init(struct multiboot_info* info);

int ata_read(unsigned long long sector, int count, void* buffer);

int ata_write(unsigned long long sector, int count, const void* buffer);

#endif
