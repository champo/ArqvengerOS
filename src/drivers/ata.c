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

unsigned long long sectors = 0L;

static void set_ports(unsigned long long sector, int count, unsigned char command);
static int poll();

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
    int i,j;
    unsigned int edi;

    edi = (unsigned int) &buffer; 

    if ( sector > sectors ) {
        return -1;
    }
    
    set_ports(sector, count, READ_COMMAND);    
    for ( i = 0; i < count; i++) {
        if ( poll() == -1 ) {
            return -1;
        }

        __asm__("mov %ax, %ds");
        __asm__("rep insw"::"c"(SIZE_WORD), "d"(DATA_PORT), "D"(edi));
        edi += (SIZE_WORD * 2);

        //400ns delay
        for ( j = 0; j< 4; j++) {
            inB(STATUS_PORT);
        }
    }
    
    return 0;

}

int ata_write(unsigned long long sector, int count, const void* buffer) {
    int i;
    unsigned int edi;

    edi = (unsigned int) &buffer;

    if ( sector > sectors ) {
        return -1;
    }
    
    set_ports(sector, count, WRITE_COMMAND); 
    
    for ( i = 0; i < count; i++) {
        poll();
        
        __asm__("mov %ax, %ds");
        __asm__("rep outsw"::"c"(SIZE_WORD), "d"(DATA_PORT), "S"(edi));
        edi += (SIZE_WORD * 2);
        //TODO Delay? Replace rep outsw with several outsw?

    }
    
    outB(COMMAND_PORT, CACHE_FLUSH);
    poll();

    return 0;
}

void set_ports(unsigned long long sector, int count, unsigned char command) {
    outB(SECTOR_COUNT_PORT, (unsigned char) count);
    outB(LBA_LOW_PORT, (unsigned char) sector);
    outB(LBA_MID_PORT, (unsigned char) (sector >> 8));
    outB(LBA_HIGH_PORT, (unsigned char) (sector >> 16));
    outB(COMMAND_PORT, command);
}

int poll() {
    unsigned char status;
    inB(CONTROL_REGISTER, status);

    while (!BSY(status)) {
        if (ERR(status) || DF(status)) {
            return -1;
        }
        inB(CONTROL_REGISTER, status);
    }
    while (!DRQ(status)){
        if (ERR(status) || DF(status)){
            return -1;
        }
        inB(CONTROL_REGISTER, status);
    }   
    return 0;
}
