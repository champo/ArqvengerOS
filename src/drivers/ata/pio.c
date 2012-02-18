#include "drivers/ata/pio.h"
#include "drivers/ata/common.h"
#include "system/io.h"

#define SIZE_WORD 256

static int poll(void);

static int checkBSY(void);

/**
 * Reads from a hard drive.
 *
 * @param sector, the start sector of the reading.
 * @param count, the number of sectors to be read.
 * @param buffer, the output of the read.
 * @return 0 if success, -1 if error.
 */
int ata_pio_read(unsigned long long sector, int count, void* buffer) {
    int i,j;

    unsigned int edi = (unsigned int) buffer;

    if ( sector + count > get_sectors() ) {
        return -1;
    }

    for ( i = 0; i < count; i++) {
        set_ports(sector + i, 1, PIO_READ_COMMAND);

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
int ata_pio_write(unsigned long long sector, int count, const void* buffer) {
    int i;

    unsigned int edi = (unsigned int) buffer;

    if ( sector + count > get_sectors() ) {
        return -1;
    }

    for ( i = 0; i < count; i++) {
        set_ports(sector + i, 1, PIO_WRITE_COMMAND);
        poll();
        __asm__("rep outsw"::"c"(SIZE_WORD), "d"(DATA_PORT), "S"((unsigned int) edi));
        edi += (SIZE_WORD * 2);
    }

    outB(COMMAND_PORT, CACHE_FLUSH);
    checkBSY();

    return 0;
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

