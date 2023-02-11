#include <arch/processor.h>
#include <panic.h>

void __panic(void) {
    dump_registers();
    halt_processor();
}
