#ifndef __DRIVERS_ATA_COMMON__
#define __DRIVERS_ATA_COMMON__

#define BASE_PORT 0x1F0

#define PORT(n) (BASE_PORT + n)

#define DATA_PORT PORT(0)
#define INFO_PORT PORT(1)
#define SECTOR_COUNT_PORT PORT(2)
#define LBA_LOW_PORT PORT(3)
#define LBA_MID_PORT PORT(4)
#define LBA_HIGH_PORT PORT(5)
#define DRIVE_PORT PORT(6)
#define COMMAND_PORT PORT(7)
#define STATUS_PORT COMMAND_PORT

#define ERR(status) (status & 0x1)
#define DRQ(status) (status & (0x1 << 3))
#define SRV(status) (status & (0x1 << 4))
#define DF(status) (status & (0x1 << 5))
#define RDY(status) (status & (0x1 << 6))
#define BSY(status) (status & (0x1 << 7))

#define CONTROL_REGISTER 0x3F6

#define MASTER 0xA0

#define PIO_READ_COMMAND 0x20
#define PIO_WRITE_COMMAND 0x30
#define DMA_READ_COMMAND 0xC8
#define DMA_WRITE_COMMAND 0xCA
#define CACHE_FLUSH 0xE7

void set_ports(unsigned long long sector, int count, unsigned char command);

unsigned long long get_sectors(void);

void set_sectors(unsigned long long sectors);

#endif
