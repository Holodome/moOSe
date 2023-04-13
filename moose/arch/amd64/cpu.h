//
// High-level handling of x86 processor
//
#pragma once

#include <arch/amd64/asm.h>

// Used with irq_save and friends to make more self-explanatory declarations
// for flags variable
typedef u64 cpuflags_t;

// Primary usage of this structure is to save registers when entering
// isr. However we also use it to save process registers when it is being
// context switched from/to in order to reduce code duplication.
struct registers_state {
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

// Size has to be checked thoroughly because we access this structure
// from within the assembly
static_assert(sizeof(struct registers_state) == 184);

#define irq_disable() cli()
#define irq_enable() sti()

#define __IRQS_DISABLED(_value) (((_value)&X86_FLAGS_IF) != 0)

static inline int irqs_disabled(void) {
    return __IRQS_DISABLED(read_cpu_flags());
}

// TODO: Linux has this strange notion of saving interrupts in local
// variable and passing varible name instead of address here.
// Although omnipresent occurence of this idiom makes it easily-recognizable,
// it is still not the most consise way. Is there actual reason for not using
// function and passing address to it?
static __nodiscard __forceinline cpuflags_t irq_save(void) {
    cpuflags_t flags = read_cpu_flags();
    cli();
    return __IRQS_DISABLED(flags);
}

static __forceinline void irq_restore(cpuflags_t flags) {
    if (flags)
        sti();
}

// Using port 80 to wait 1us
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

void init_process_registers(struct registers_state *regs, void (*fn)(void *),
                            void *arg, u64 stack_end);
