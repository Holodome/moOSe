#include "moose.h"

#include "kprint.h"
#include "vga.h"

void _kmain(void) {
    kcls();
    kprintf("running moOSe kernel\n");

    for (;;)
        ;
}
