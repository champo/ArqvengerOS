#include "system/process/scheduler.h"
#include "type.h"

static struct Process* list[20] = {0};

static int current = -1;

static int turns;

static void choose_next(void);

void scheduler_add(struct Process* process) {
    for (size_t i = 0; i < 20; i++) {
        if (list[i] == NULL) {
            list[i] = process;
            break;
        }
    }
}

void scheduler_do(void) {

    if (current != -1) {
        __asm__ __volatile ("mov %%ebp, %0":"=r"(list[current]->mm.esp)::);
    }

    choose_next();

    if (current != -1) {
        __asm__ __volatile__ ("mov %0, %%ebp"::"r"(list[current]->mm.esp));
    }
}

void choose_next(void) {
    turns = 0;
    do {
        current = (current + 1) % 20;
        if (++turns == 21) {
            current = -1;
            return;
        }
    } while (list[current] == NULL);
}

void scheduler_remove(struct Process* process) {
    for (size_t i = 0; i < 20; i++) {
        if (list[i] == process) {
            list[i] = NULL;
            return;
        }
    }
}

