#include <arch/cpu.h>
#include <panic.h>

void __panic(void) {
    dump_registers();
    ___panic();
}

void ___panic(void) {
    halt_cpu();
}
