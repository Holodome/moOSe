#include <arch/amd64/asm.h>
#include <arch/cpu.h>
#include <kstdio.h>

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

static void get_registers(struct registers *regs) {
    asm("push %%rax\n"
        "mov %0, %%rax\n"
        "mov %%rdi, 0x0(%%rax)\n"
        "mov %%rsi, 0x8(%%rax)\n"
        "mov %%rbp, 0x10(%%rax)\n"
        "mov %%rsp, 0x18(%%rax)\n"
        "mov %%rbx, 0x20(%%rax)\n"
        "mov %%rdx, 0x28(%%rax)\n"
        "mov %%rcx, 0x30(%%rax)\n"
        "mov %%rax, 0x38(%%rax)\n"
        "mov %%r8, 0x40(%%rax)\n"
        "mov %%r9, 0x48(%%rax)\n"
        "mov %%r10, 0x50(%%rax)\n"
        "mov %%r11, 0x58(%%rax)\n"
        "mov %%r12, 0x60(%%rax)\n"
        "mov %%r13, 0x68(%%rax)\n"
        "mov %%r14, 0x70(%%rax)\n"
        "mov %%r15, 0x78(%%rax)\n"
        "push %%rbx\n"
        "lea 0(%%rip), %%rbx\n"
        "mov %%rbx, 0x80(%%rax)\n"
        "pushf\n"
        "pop %%rbx\n"
        "mov %%rbx, 0x88(%%rax)\n"
        "mov %%cr0, %%rbx\n"
        "mov %%rbx, 0x90(%%rax)\n"
        "mov %%cr2, %%rbx\n"
        "mov %%rbx, 0x98(%%rax)\n"
        "mov %%cr3, %%rbx\n"
        "mov %%rbx, 0xa0(%%rax)\n"
        "pop %%rbx\n"
        "pop %%rax\n"
        :
        : "m"(regs)
        : "memory");
}

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

void halt_cpu(void) {
    cli();
    hlt();
}

void set_stack(u64 sp, u64 old_stack_base) {
    asm volatile("movq %0, %%rdi\n"
                 "movq %1, %%rcx\n"
                 "movq %%rsp, %%rsi\n"
                 "subq %%rsi, %%rcx\n"
                 "subq %%rcx, %%rdi\n"
                 "movq %%rdi, %%rbx\n"
                 "rep movsb\n"
                 "movq %%rbx, %%rsp\n"
                 "movq %%rsp, %%rbp\n"
                 :
                 : "r"(sp), "r"(old_stack_base)
                 : "memory");
}

