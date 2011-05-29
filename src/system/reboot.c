#include "system/reboot.h"
#include "system/common.h"
#include "system/io.h"

#define INTERFACE_PORT 0x64
#define IO_PORT 0x60

#define RESET_CODE 0xFE

#define KDATA_FLAG 0x1
#define UDATA_FLAG 0x2


/**
 * Reboot the system.
 *
 * It restarts the system by telling the keyboard driver to tell the  CPU
 * to reset itself.
 */
void reboot(void) {
    char aux;

    // We use the keyboard driver to reset the CPU
    disableInterrupts();

    // We need to consume everything in the keyboard buffers
    do {

        aux = inB(INTERFACE_PORT);
        if (aux & KDATA_FLAG) {
            inB(IO_PORT);
        }
    } while (aux & UDATA_FLAG);

    // This should tell the keyboard driver to signal the CPU for reset
    outB(INTERFACE_PORT, RESET_CODE);

    halt();
}
