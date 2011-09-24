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

unsigned long long sectors = 0L;

void ata_init(struct multiboot_info* info) {

    if (info->flags & (0x1 << 7)) {

        if (info->drives_length) {

            struct DriveInfo* drive = (struct DriveInfo*) info->drives_addr;
            if (drive->number == 0x80 && drive->mode) {
                sectors = drive->cylinders * drive->heads * drive->sectors;
                if (sectors > (1 << 28)) {
                    sectors = 1 << 28;
                }
            }
        }
    }
}

