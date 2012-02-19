#ifndef __DRIVERS_ATA_DMA__
#define __DRIVERS_ATA_DMA__

#include "drivers/pci.h"

void ata_dma_irq(void);

int ata_dma_read(unsigned long long sector, int count, void* buffer);

int ata_dma_write(unsigned long long sector, int count, const void* buffer);

void ata_dma_init(struct PCIDevice* device);

#endif
