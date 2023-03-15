#include <arch/amd64/asm.h>
#include <arch/cpu.h>
#include <kstdio.h>
#include <kthread.h>

struct registers {
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

    u64 rip;
    u64 rflags;
    u64 cr0;
    u64 cr2;
    u64 cr3;
};

static_assert(sizeof(struct registers) == 0xa8);

extern void get_registers(struct registers *regs) __attribute__((used));
extern void set_stack(u64 sp, u64 old_stack_base) __attribute__((used));

static void print_registers(const struct registers *r) {
    kprintf("rip: %#018llx rflags: %#018llx\n", r->rip, r->rflags);
    kprintf("rdi: %#018llx rsi: %#018llx rbp: %#018llx\n", r->rdi, r->rsi,
            r->rbp);
    kprintf("rsp: %#018llx rbx: %#018llx rdx: %#018llx\n", r->rsp, r->rbx,
            r->rdx);
    kprintf("rcx: %#018llx rax: %#018llx r8:  %#018llx\n", r->rcx, r->rax,
            r->r8);
    kprintf("r9:  %#018llx r10: %#018llx r11: %#018llx\n", r->r9, r->r10,
            r->r11);
    kprintf("r12: %#018llx r13: %#018llx r14: %#018llx\n", r->r12, r->r13,
            r->r14);
    kprintf("r15: %#018llx\n", r->r15);
    kprintf("cr0: %#018llx cr2: %#018llx cr3: %#018llx\n", r->cr0, r->cr2,
            r->cr3);
}

void dump_registers(void) {
    struct registers regs;
    get_registers(&regs);
    print_registers(&regs);
}
