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

#define READ_COMMAND 0x20
#define WRITE_COMMAND 0x30
#define CACHE_FLUSH 0xE7

#define SIZE_WORD 256

static unsigned long long sectors = 0L;

static void set_ports(unsigned long long sector, int count, unsigned char command);
static int poll(void);
static int checkBSY(void);

/**
 * Checks if an ata driver has been initialized.
 *
 * @return 1 if true, 0 if not.
 */
int ata_has_drive(void) {
    return !!sectors;
}

/**
 * Initialize an ata driver.
 *
 * @param info, the multibot structure given by GRUB.
 */
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

/**
 * Reads from a hard drive.
 *
 * @param sector, the start sector of the reading.
 * @param count, the number of sectors to be read.
 * @param buffer, the output of the read.
 * @return 0 if success, -1 if error.
 */
int ata_read(unsigned long long sector, int count, void* buffer) {
    int i,j;

    unsigned int edi = (unsigned int) buffer;

    if ( sector + count > sectors ) {
        return -1;
    }

    for ( i = 0; i < count; i++) {
        set_ports(sector + i, 1, READ_COMMAND);

        if ( poll() == -1 ) {
            return -1;
        }

        __asm__("rep insw"::"c"(SIZE_WORD), "d"(DATA_PORT), "D"((unsigned int) edi));
        edi += (SIZE_WORD * 2);

        //400ns delay
        for ( j = 0; j< 4; j++) {
            inB(STATUS_PORT);
        }
    }

    return 0;

}

/**
 * Writes into a hard drive.
 *
 * @param sector, the start sector of the writing.
 * @param count, the number of sectors to be written.
 * @param buffer, the source to be written.
 * @return 0 if success, -1 if error.
 */
int ata_write(unsigned long long sector, int count, const void* buffer) {
    int i;

    unsigned int edi = (unsigned int) buffer;

    if ( sector + count > sectors ) {
        return -1;
    }

    for ( i = 0; i < count; i++) {
        set_ports(sector + i, 1, WRITE_COMMAND);
        poll();
        __asm__("rep outsw"::"c"(SIZE_WORD), "d"(DATA_PORT), "S"((unsigned int) edi));
        edi += (SIZE_WORD * 2);
    }

    outB(COMMAND_PORT, CACHE_FLUSH);
    checkBSY();

    return 0;
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

/**
 * Waits until the hard drive is ready to be operated or tells if an error occured.
 *
 * @return 0 if the drive is ready, -1 if an error was detected.
 */
int poll(void) {
    unsigned char status;

    if (checkBSY() != 0) {
        return -1;
    }

    status = inB(STATUS_PORT);

    while (!DRQ(status)) {
        if (ERR(status) || DF(status)) {
            return -1;
        }
        status = inB(STATUS_PORT);
    }

    return 0;
}

/**
 * Checks and waits until the hard drive status is not busy.
 *
 * @return 0 if success, -1 if an error was detected in the drive.
 */
int checkBSY(void) {

    unsigned char status = inB(STATUS_PORT);

    while (BSY(status)) {
        if (ERR(status) || DF(status)) {
            return -1;
        }
        status = inB(STATUS_PORT);
    }

    return 0;
}
