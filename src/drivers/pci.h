#ifndef __DRIVER_PCI__
#define __DRIVER_PCI__

struct PCIDevice {
    unsigned int bus;
    unsigned int device;
    unsigned int func;
};

int pci_find_device(struct PCIDevice* device, unsigned char class, unsigned char subclass);

/**
 * Write a value onto the PCI configuration for a given device.
 *
 * @param device The device to write to.
 * @param offset The offset in the config table (must be dword aligned)
 * @param value The value to set
 */
void pci_set_config(struct PCIDevice* device, unsigned short offset, unsigned int value);

unsigned int pci_read_config(struct PCIDevice* device, unsigned short offset);

#endif
