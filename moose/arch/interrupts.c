#include <arch/amd64/idt.h>
#include <arch/interrupts.h>
#include <assert.h>
#include <kstdio.h>
#include <sched/locks.h>
#include <sched/scheduler.h>

static struct {
    struct list_head isr_lists[256];
    spinlock_t lock;
} interrupts;

#define __abort_in_handler(...)                                                \
    do {                                                                       \
        kprintf(__VA_ARGS__);                                                  \
        kprintf("\n");                                                         \
        print_registers_state(r);                                              \
        halt_cpu();                                                            \
    } while (0)

// TODO: These should send signal to current process if we are
// in user mode and __abort_in_handler the kernel otherwise
// TODO: Print registers_state instead of registers that __abort_in_handler gets
static irqresult_t division_by_zero_handler(void *dev __unused,
                                            const struct registers_state *r) {
    __abort_in_handler("division by zero");
}

// NOTE: Because clang-format is f***ing stupid it does not understand
// macros expanding to functions and completely messes up surrounding
// indentation
// clang-format on

static irqresult_t
illegal_instruction_handler(void *dev __unused,
                            const struct registers_state *r) {
    __abort_in_handler("illegal instruction");
}

static irqresult_t page_fault_handler(void *dev __unused,
                                      const struct registers_state *r) {
    __abort_in_handler("page fault at address: %#018lx", read_cr2());
}

// TODO: This has to be somewhere x86-specific
static irqresult_t debug_exception_handler(void *dev __unused,
                                           const struct registers_state *r) {
    __abort_in_handler("debug exception");
}

static irqresult_t nmi_handler(void *arg __unused,
                               const struct registers_state *r) {
    __abort_in_handler("nmi");
}

static irqresult_t breakpoint_handler(void *dev __unused,
                                      const struct registers_state *r) {
    __abort_in_handler("breakpoint");
}

static irqresult_t into_handler(void *dev __unused,
                                const struct registers_state *r) {
    __abort_in_handler("into");
}

static irqresult_t out_of_bounds_handler(void *dev __unused,
                                         const struct registers_state *r) {
    __abort_in_handler("out of bounds");
}

static irqresult_t no_fpu_handler(void *dev __unused,
                                  const struct registers_state *r) {
    __abort_in_handler("no fpu");
}

static irqresult_t double_fault_handler(void *dev __unused,
                                        const struct registers_state *r) {
    __abort_in_handler("double fault");
}

static irqresult_t
fpu_segment_overrun_handler(void *dev __unused,
                            const struct registers_state *r) {
    __abort_in_handler("fpu segment overrun");
}

static irqresult_t bad_tss_handler(void *dev __unused,
                                   const struct registers_state *r) {
    __abort_in_handler("bad tss segment");
}

static irqresult_t
segment_not_present_handler(void *dev __unused,
                            const struct registers_state *r) {
    __abort_in_handler("segment not present");
}

static irqresult_t stack_fault_handler(void *dev __unused,
                                       const struct registers_state *r) {
    __abort_in_handler("stack fault");
}

static irqresult_t
general_protection_fault_handler(void *dev __unused,
                                 const struct registers_state *r) {
    __abort_in_handler("general protection fault");
}

static void register_exception_handler(struct interrupt_handler *handler) {
    list_add(&handler->list, interrupts.isr_lists + handler->number);
}

void init_interrupts(void) {
    for (size_t i = 0; i < ARRAY_SIZE(interrupts.isr_lists); ++i) {
        init_list_head(&interrupts.isr_lists[i]);
    }

    init_spin_lock(&interrupts.lock);
#define __DEFINE_HANDLER(_num, _name, _fun)                                    \
    do {                                                                       \
        static struct interrupt_handler irq = {                                \
            .number = _num, .name = _name, .handle_interrupt = _fun};          \
        register_exception_handler(&irq);                                      \
    } while (0)
    __DEFINE_HANDLER(0, "zero division", division_by_zero_handler);
    __DEFINE_HANDLER(1, "debug", debug_exception_handler);
    __DEFINE_HANDLER(2, "nmi", nmi_handler);
    __DEFINE_HANDLER(3, "brk", breakpoint_handler);
    __DEFINE_HANDLER(4, "into", into_handler);
    __DEFINE_HANDLER(5, "bound", out_of_bounds_handler);
    __DEFINE_HANDLER(6, "ud2", illegal_instruction_handler);
    __DEFINE_HANDLER(7, "no fpu", no_fpu_handler);
    __DEFINE_HANDLER(8, "double fault", double_fault_handler);
    __DEFINE_HANDLER(9, "fpu overrun", fpu_segment_overrun_handler);
    __DEFINE_HANDLER(10, "invalid tss", bad_tss_handler);
    __DEFINE_HANDLER(11, "invalid segment", segment_not_present_handler);
    __DEFINE_HANDLER(12, "stack fault", stack_fault_handler);
    __DEFINE_HANDLER(13, "gpf", general_protection_fault_handler);
    __DEFINE_HANDLER(14, "page fault", page_fault_handler);
#undef __DEFINE_HANDLER
}

void isr_handler(struct registers_state *regs) {
    unsigned no = regs->isr_number;
    if (spin_trylock(&interrupts.lock)) {
        struct interrupt_handler *handler;
        list_for_each_entry(handler, &interrupts.isr_lists[no], list) {
            irqresult_t result = handler->handle_interrupt(handler->dev, regs);
            if (result == IRQ_HANDLED)
                break;
        }
        spin_unlock(&interrupts.lock);
    }

    eoi(no);
    sti();

    if (eat_should_invoke_scheduler())
        schedule();
}

void enable_interrupt(struct interrupt_handler *handler) {
    spin_lock(&interrupts.lock);
    list_add(&handler->list, &interrupts.isr_lists[handler->number + 32]);
    spin_unlock(&interrupts.lock);
}

void disable_interrupt(struct interrupt_handler *handler) {
    spin_lock(&interrupts.lock);
    list_remove(&handler->list);
    spin_unlock(&interrupts.lock);
}
