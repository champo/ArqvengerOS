#include "drivers/ata.h"
#include "type.h"

struct DriveInfo {
    size_t len;
    unsigned char number;
    unsigned char mode;
    unsigned short cylinders;
    unsigned char heads;
    unsigned char sectors;
    unsigned short ports[];
};

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

unsigned long long sectors = 0L;

void ata_init(struct multiboot_info* info) {

    if ((info->flags & (0x1 << 7)) && info->drives_length) {

        struct DriveInfo* drive = (struct DriveInfo*) info->drives_addr;
        if (drive->number == 0x80 && drive->mode) {
            sectors = drive->cylinders * drive->heads * drive->sectors;
            if (sectors > (1 << 28)) {
                sectors = 1 << 28;
            }

            outB(DRIVE_PORT, MASTER);
        }
    }
}

int ata_read(unsigned long long sector, int count, void* buffer) {
}

int ata_write(unsigned long long sector, int count, const void* buffer) {
}

