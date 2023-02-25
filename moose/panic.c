#include <arch/cpu.h>
#include <panic.h>

void __panic(void) {
    dump_registers();
    halt_cpu();
}