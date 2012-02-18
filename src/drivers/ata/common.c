#include "drivers/ata/common.h"
#include "system/io.h"

static unsigned long long sectorCount = 0L;

unsigned long long get_sectors(void) {
    return sectorCount;
}

void set_sectors(unsigned long long sectors) {
    sectorCount = sectors;
}

/**
 * Sets the ports before an opperation into a hard drive
 *
 * @param sector, the starting sector of the opperation.
 * @param count, the number of sectors to be used.
 * @param command, defines the operation.
 */
void set_ports(unsigned long long sector, int count, unsigned char command) {
    outB(DRIVE_PORT, 0xE0 | ((sector >> 24) & 0x0F));
    outB(SECTOR_COUNT_PORT, (unsigned char) count);
    outB(LBA_LOW_PORT, (unsigned char) sector);
    outB(LBA_MID_PORT, (unsigned char) (sector >> 8));
    outB(LBA_HIGH_PORT, (unsigned char) (sector >> 16));
    outB(COMMAND_PORT, command);
}

