#include <moose/arch/cpu.h>
#include <moose/panic.h>

void __panic(void) {
    dump_registers();
    halt_cpu();
}

void ___panic(void) {
    halt_cpu();
}
