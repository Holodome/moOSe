#include "moose.h"

#include "interrupt.h"
#include "keyboard.h"
#include "kprint.h"
#include "vga.h"

void _kmain(void) {
    init_interrupts();

    init_keyboard();

    kcls();
    kprintf("running moOSe kernel\n");

    for (;;)
        ;
}
