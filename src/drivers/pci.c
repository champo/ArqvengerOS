#include "drivers/pci.h"
#include "system/io.h"

#define ADDRESS_PORT 0xCF8
#define DATA_PORT 0xCFC

#define VENDOR_OFFSET 0x0
#define HEADER_TYPE_OFFSET 0x0C
#define CLASS_OFFSET 0x08

static unsigned int data_address(struct PCIDevice* device, unsigned short offset);

static int is_device_valid(struct PCIDevice* device);

static int has_class(struct PCIDevice* device, unsigned char class, unsigned char subclass);

static int is_multi(struct PCIDevice* device);

int pci_find_device(struct PCIDevice* device, unsigned char class, unsigned char subclass) {

    struct PCIDevice dev = {0, 0, 0};
    int done = 0;

    for (dev.bus = 0; dev.bus < 3; dev.bus++) {
        for (dev.device = 0, done = 0; !done; dev.device++) {
            for (dev.func = 0;; dev.func++) {

                if (is_device_valid(&dev)) {

                    if (has_class(&dev, class, subclass)) {
                        *device = dev;
                        return 0;
                    }

                    if (!is_multi(&dev)) {
                        break;
                    }

                } else {
                    done = 1;
                    break;
                }
            }
        }
    }

    return -1;
}

int is_multi(struct PCIDevice* device) {
    return pci_read_config(device, HEADER_TYPE_OFFSET) & (0x1 << 23);
}

int has_class(struct PCIDevice* device, unsigned char class, unsigned char subclass) {
    unsigned int id = pci_read_config(device, CLASS_OFFSET) >> 16;
    return (id & 0xFFFF) == (subclass | (class << 8));
}

void pci_set_config(struct PCIDevice* device, unsigned short offset, unsigned int value) {
    outD(ADDRESS_PORT, data_address(device, offset));
    outD(DATA_PORT, value);
}

int is_device_valid(struct PCIDevice* device) {
    unsigned int vendor = pci_read_config(device, VENDOR_OFFSET);
    return (0xFFFF & vendor) != 0xFFFF;
}

unsigned int pci_read_config(struct PCIDevice* device, unsigned short offset) {
    outD(ADDRESS_PORT, data_address(device, offset));
    return inD(DATA_PORT);
}

unsigned int data_address(struct PCIDevice* device, unsigned short offset) {
    return (unsigned int) (
            ((device->bus & 0xFF) << 16)
            | ((device->device & 0x1F) << 11)
            | ((device->func & 0x07) << 8)
            | ((unsigned int) offset & 0xFC)
            | (1u << 31)
    );
}
