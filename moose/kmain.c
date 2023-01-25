#include "moose.h"

#include "kprint.h"
#include "vga.h"
#include "interrupt.h"

void _kmain(void) {
    init_interrupts();

    kcls();
    kprintf("running moOSe kernel\n");

    for (;;)
        ;
}
