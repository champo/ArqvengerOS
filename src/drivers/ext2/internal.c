#include "drivers/ext2/internal.h"

int read_sectors(struct ext2* fs, unsigned long long sector, size_t sectors, void* buffer) {
    return ata_read(fs->firstSector + sector, sectors, buffer);
}

