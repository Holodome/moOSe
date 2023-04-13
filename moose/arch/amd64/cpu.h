#pragma once

#include <arch/amd64/asm.h>

typedef u64 cpuflags_t;

struct process_registers {
    u64 rsp;
    u64 rip;
    u64 rflags;

    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rbx;
    u64 rdx;
    u64 rcx;
    u64 rax;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
};

struct isr_context {
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rsp;
    u64 rbx;
    u64 rdx;
    u64 rcx;
    u64 rax;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;

    u64 isr_number;
    u64 exception_code;

    u64 rip;
    u64 cs;
    u64 rflags;
    u64 ursp;
    u64 uss;
};

static_assert(sizeof(struct isr_context) == 184);

#define irq_disable() cli()
#define irq_enable() sti()

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
        if (_flags & X86_FLAGS_IF)                                             \
            sti();                                                             \
    } while (0)

static inline void io_wait(void) {
    port_out32(0x80, 0);
}

#define halt_cpu()                                                             \
    do {                                                                       \
        cli();                                                                 \
        hlt();                                                                 \
        __builtin_unreachable();                                               \
    } while (0)
#define wait_for_int() hlt()
#define spinloop_hint() pause()

void init_process_registers(struct process_registers *regs, void (*fn)(void *),
                            void *arg, u64 stack_end);
