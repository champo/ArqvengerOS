#include "system/task.h"
#include "system/mm/allocator.h"
#include "system/gdt.h"

static struct TaskState* ts;

struct TaskState* task_create_tss(void) {
    ts = allocPages(1);

    ts->iopb = sizeof(struct TaskState);

    ts->cs = KERNEL_CODE_SEGMENT;
    ts->ds = KERNEL_DATA_SEGMENT;
    ts->es = KERNEL_DATA_SEGMENT;
    ts->fs = KERNEL_DATA_SEGMENT;
    ts->gs = KERNEL_DATA_SEGMENT;

    ts->ss = KERNEL_DATA_SEGMENT;
    ts->ss0 = KERNEL_DATA_SEGMENT;

    return ts;
}

void task_load(struct Process* p) {
    ts->esp0 = p->mm.kernelStack;
}

