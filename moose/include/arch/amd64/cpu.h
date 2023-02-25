#pragma once

#include <arch/amd64/asm.h>

static inline void irq_disable(void) { cli(); }
static inline void irq_enable(void) { sti(); }
static inline int irqs_disabled(void) { return (read_cpu_flags() & X86_FLAGS_IF) != 0; }
static inline void irq_save(volatile unsigned long *dst) {
    *dst = read_cpu_flags();
    cli();
}

static inline void irq_restore(unsigned long src) {
    if (src & X86_FLAGS_IF)
        sti();
}
