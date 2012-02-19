#include "drivers/ata/common.h"
#include "drivers/ata/dma.h"
#include "debug.h"
#include "drivers/ata.h"
#include "system/io.h"
#include "system/mm/allocator.h"
#include "system/process/table.h"
#include "system/scheduler.h"

#define DMA_CONTROL_PORT (bar4)
#define DMA_STATUS_PORT (bar4 + 0x2)
#define DMA_PRDT_PORT (bar4 + 0x4)

#define START_BIT 0x1
#define WRITE_BIT 0x8
#define INTERRUPT_BIT 0x4

static unsigned int bar4;

struct PRD {
    unsigned int address;
    unsigned short bytes;
    int unused:15;
    unsigned int eot:1;
};

static struct PRD* prdt;

struct DiskTask {
    unsigned long long sector;
    int count;
    void* buffer;
    int write;
    int running;

    struct Process* owner;

    struct DiskTask* next;
    struct DiskTask* prev;
};

static struct DiskTask* firstTask = NULL;
static struct DiskTask* lastTask = NULL;

static struct DiskTask* unused = NULL;

static void enqueue_task(unsigned long long sector, int count, void* buffer, int write);

static void execute_task(void);

static void reset_controller(void);

void ata_dma_init(struct PCIDevice* device) {
    bar4 = pci_read_config(device, 0x20) & ~0x3;
    prdt = allocPages(1);
    outD(DMA_PRDT_PORT, prdt);
}

void reset_controller(void) {
    log_debug("Reset controller\n");
    outB(DMA_CONTROL_PORT, 0x0);
    outB(DMA_STATUS_PORT, INTERRUPT_BIT);

    log_debug("Reset status %u, %u\n", inB(DMA_STATUS_PORT), inB(STATUS_PORT));
}

void ata_dma_irq(void) {

    if ((inB(DMA_STATUS_PORT) & INTERRUPT_BIT) == 0) {
        log_debug("Not!\n");
        return;
    }

    log_debug("Handling an IRQ\n");
    reset_controller();

    if (!firstTask) {
        return;
    }

    log_debug("Waking %u\n", firstTask->owner->pid);
    firstTask->owner->schedule.ioWait = 0;
    process_table_unblock(firstTask->owner);

    struct DiskTask* task = firstTask;
    if (task->next) {
        task->next->prev = NULL;
        firstTask = task->next;

        log_debug("%u\n", firstTask);
        execute_task();
    } else {
        firstTask = NULL;
        lastTask = NULL;
    }

    task->next = unused;
    unused = task;
}

void enqueue_task(unsigned long long sector, int count, void* buffer, int write) {

    log_debug("Enqueueing task %u %u %u %u %d\n", scheduler_current(), (unsigned int)sector, count, buffer, write);
    struct DiskTask* task;
    if (unused) {
        task = unused;
        unused = task->next;
    } else {
        task = kalloc(sizeof(struct DiskTask));
    }

    task->sector = sector;
    task->count = count;
    task->buffer = buffer;
    task->write = write;
    task->owner = scheduler_current();
    task->running = 0;

    task->next = NULL;
    task->prev = lastTask;

    if (lastTask) {
        lastTask->next = task;
    }

    lastTask = task;

    if (!firstTask) {
        firstTask = task;
        execute_task();
    }

    log_debug("Blocking %u\n", task->owner->pid);
    task->owner->schedule.ioWait = 1;
    process_table_block(task->owner);

    scheduler_do();
}

void execute_task(void) {

    unsigned char command;
    unsigned char write;

    if (firstTask->write) {
        command = DMA_WRITE_COMMAND;
        write = WRITE_BIT;
    } else {
        command = DMA_READ_COMMAND;
        write = 0;
    }

    prdt[0].address = (unsigned int) firstTask->buffer;
    prdt[0].bytes = firstTask->count * SECTOR_SIZE;
    prdt[0].eot = 1;

    unsigned char status = inB(DMA_STATUS_PORT);
    log_debug("Status pre-execute %u\n", status);
    outB(DMA_STATUS_PORT, status & INTERRUPT_BIT);

    outB(DMA_CONTROL_PORT, write);

    set_ports(firstTask->sector, firstTask->count, command);

    outB(DMA_CONTROL_PORT, write | START_BIT);
    firstTask->running = 1;

    log_debug("Status post execute %u\n", inB(DMA_STATUS_PORT));
}


int ata_dma_read(unsigned long long sector, int count, void* buffer) {

    enqueue_task(sector, count, buffer, 0);
    return count;
}

int ata_dma_write(unsigned long long sector, int count, const void* buffer) {

    enqueue_task(sector, count, buffer, 1);
    return count;
}

