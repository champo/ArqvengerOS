#include "drivers/ata.h"

#include "drivers/ata/common.h"
#include "drivers/ata/pio.h"
#include "drivers/ata/dma.h"

#include "type.h"
#include "drivers/pci.h"
#include "system/kprintf.h"
#include "system/io.h"
#include "system/interrupt.h"

struct DriveInfo {
    size_t len;
    unsigned char number;
    unsigned char mode;
    unsigned short cylinders;
    unsigned char heads;
    unsigned char sectors;
    unsigned short ports[];
};

static int (*read)(unsigned long long, int, void*) = NULL;
static int (*write)(unsigned long long, int, const void*) = NULL;
static void (*irq)(void) = NULL;

static int dmaCapable = 0;

/**
 * Checks if an ata driver has been initialized.
 *
 * @return 1 if true, 0 if not.
 */
int ata_has_drive(void) {
    return !!get_sectors();
}

/**
 * Initialize an ata driver.
 *
 * @param info, the multibot structure given by GRUB.
 */
void ata_init(struct multiboot_info* info) {

    unsigned long long sectors;
    if ((info->flags & (0x1 << 7)) && info->drives_length) {

        struct DriveInfo* drive = (struct DriveInfo*) info->drives_addr;
        if (drive->number == 0x80 && drive->mode) {
            sectors = drive->cylinders * drive->heads * drive->sectors;
            if (sectors > (1 << 28)) {
                sectors = 1 << 28;
            }
            set_sectors(sectors);

            outB(DRIVE_PORT, MASTER);

            struct PCIDevice device;
            if (pci_find_device(&device, 0x01, 0x01) != -1) {
                dmaCapable = 1;
                ata_dma_init(&device);
            } else {
                dmaCapable = 0;
            }

            read = ata_pio_read;
            write = ata_pio_write;
            irq = NULL;
        }
    }
}

int ata_read(unsigned long long sector, int count, void* buffer) {

    if (read) {
        return read(sector, count, buffer);
    }

    return -1;
}

void ata_irq(void) {

    if (irq) {
        irq();
    }
}

int ata_write(unsigned long long sector, int count, const void* buffer) {

    if (write) {
        return write(sector, count, buffer);
    }

    return -1;
}

void ata_enable_dma(void) {

    if (dmaCapable) {
        read = ata_dma_read;
        write = ata_dma_write;
        irq = ata_dma_irq;

        interrupt_enable_disk();
    }
}

void ata_disable_dma(void) {
    read = ata_pio_read;
    write = ata_pio_write;
    irq = NULL;

    interrupt_disable_disk();
}
