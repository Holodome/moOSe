//
// High-level handling of x86 processor
//
#pragma once

#include <moose/arch/amd64/asm.h>
#include <moose/arch/atomic.h>

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

static __forceinline void irq_disable(void) {
    cli();
}
static __forceinline void irq_enable(void) {
    sti();
}

#define __IRQS_DISABLED(_value) (((_value)&X86_FLAGS_IF) != 0)

static inline int irqs_disabled(void) {
    return __IRQS_DISABLED(read_cpu_flags());
}

// NOTE: Linux uses strange-looking idiom where irq_save is implemented as
// macro. This is due to some strangeness of SPARC architecture which
// we have to plans to support so irq_save is implement in more
// C-y way
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
static __forceinline void io_wait(void) {
    port_out32(0x80, 0);
}

static __forceinline __noreturn void halt_cpu(void) {
    cli();
    hlt();
    __builtin_unreachable();
}

static __forceinline void wait_for_int(void) {
    hlt();
}
static __forceinline void spinloop_hint(void) {
    pause();
}

void init_process_registers(struct registers_state *regs, void (*fn)(void *),
                            void *arg, u64 stack_end);

void print_registers_state(const struct registers_state *state);

struct percpu {
    // we need to add this thingy to be able to work with this structure
    // as regular pointer instead of constantly going through offsetof
    struct percpu *this;
    struct process *current;
    atomic_t preempt_count;
    // this is not atomic because it is accessed only in non-interruptible
    // context
    int should_invoke_scheduler;

    u64 user_stack;
    u64 kernel_stack;
};

void init_percpu(void);
void init_cpu(void);

static __forceinline __nodiscard struct percpu *get_percpu(void) {
    return read_gs_ptr(offsetof(struct percpu, this));
}

static __forceinline __nodiscard struct process *get_current(void) {
    return read_gs_ptr(offsetof(struct percpu, current));
}

static __forceinline void set_current(struct process *current) {
    write_gs_ptr(offsetof(struct percpu, current), current);
}

static __forceinline int get_preempt_count(void) {
    struct percpu *percpu = get_percpu();
    return atomic_read(&percpu->preempt_count);
}

static __forceinline void preempt_disable(void) {
    struct percpu *percpu = get_percpu();
    atomic_inc(&percpu->preempt_count);
}

static __forceinline void preempt_enable(void) {
    struct percpu *percpu = get_percpu();
    atomic_dec(&percpu->preempt_count);
}

static __forceinline void set_invoke_scheduler_async(void) {
    write_gs_int(offsetof(struct percpu, should_invoke_scheduler), 1);
}

static __forceinline int eat_should_invoke_scheduler(void) {
    int should_invoke_scheduler =
        read_gs_int(offsetof(struct percpu, should_invoke_scheduler));
    if (should_invoke_scheduler) {
        write_gs_int(offsetof(struct percpu, should_invoke_scheduler), 0);
    }

    return should_invoke_scheduler;
}
