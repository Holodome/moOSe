#include <panic.h>
#include <arch/cpu.h>

void __panic(void) {
    dump_registers();
    halt_cpu();
}

void ___panic(void) {
    halt_cpu();
}
