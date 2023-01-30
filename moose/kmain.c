#include "kstdio.h"

__attribute__((noreturn)) void kmain(void) {
    kputs("running moOSe kernel");
    kprintf("build %s %s\n", __DATE__, __TIME__);
    kprintf("%#5o", 108);
    for (;;)
        ;
}
