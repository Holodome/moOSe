#include "moose.h"

#include "vga.h"

void _kmain(void) {
    kcls();
    kputs("hello, moOSe\n");

    for (;;);
}
