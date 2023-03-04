#pragma once

#include <arch/amd64/asm.h>

static inline void irq_disable(void) { cli(); }
static inline void irq_enable(void) { sti(); }
static inline int irqs_disabled(void) {
    return (read_cpu_flags() & X86_FLAGS_IF) != 0;
}

#define irq_save(_flags)                                                       \
    do {                                                                       \
        _flags = read_cpu_flags();                                             \
        cli();                                                                 \
    } while (0)

#define irq_restore(_flags)                                                    \
    do {                                                                       \
        if (_flags & X86_FLAGS_IF) sti();                                      \
    } while (0)

#define halt_cpu()                                                             \
    do {                                                                       \
        cli();                                                                 \
        hlt();                                                                 \
        __builtin_unreachable();                                               \
    } while (0)
#define wait_for_int() hlt()
#define spinlock_hint() pause()
