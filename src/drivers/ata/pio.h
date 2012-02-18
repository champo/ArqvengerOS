#ifndef __DRIVERS_ATA_PIO__
#define __DRIVERS_ATA_PIO__

int ata_pio_write(unsigned long long sector, int count, const void* buffer);

int ata_pio_read(unsigned long long sector, int count, void* buffer);

#endif
