#include <arch/amd64/asm.h>
#include <arch/cpu.h>
#include <assert.h>
#include <kstdio.h>
#include <mm/kmalloc.h>
#include <sched/process.h>

#define MSR_EFER 0xc0000080
#define MSR_STAR 0xc0000081
#define MSR_LSTAR 0xc0000082
#define MSR_SFMASK 0xc0000084
#define MSR_FS_BASE 0xc0000100
#define MSR_GS_BASE 0xc0000101
#define MSR_IA32_EFER 0xc0000080

struct process;

struct debug_registers {
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

static_assert(sizeof(struct debug_registers) == 0xa8);

static __noinline __naked void
get_registers(struct debug_registers *r __unused) {
    asm volatile("pushq %rax\n"
                 "movq %rdi, %rax\n"
                 "movq %rdi, 0x0(%rax)\n"
                 "movq %rsi, 0x8(%rax)\n"
                 "movq %rbp, 0x10(%rax)\n"
                 "movq %rsp, 0x18(%rax)\n"
                 "movq %rbx, 0x20(%rax)\n"
                 "movq %rdx, 0x28(%rax)\n"
                 "movq %rcx, 0x30(%rax)\n"
                 "movq %rax, 0x38(%rax)\n"
                 "movq %r8, 0x40(%rax)\n"
                 "movq %r9, 0x48(%rax)\n"
                 "movq %r10, 0x50(%rax)\n"
                 "movq %r11, 0x58(%rax)\n"
                 "movq %r12, 0x60(%rax)\n"
                 "movq %r13, 0x68(%rax)\n"
                 "movq %r14, 0x70(%rax)\n"
                 "movq %r15, 0x78(%rax)\n"
                 "pushq %rbx\n"
                 "leaq 0(%rip), %rbx\n"
                 "movq %rbx, 0x80(%rax)\n"
                 "pushfq\n"
                 "popq %rbx\n"
                 "movq %rbx, 0x88(%rax)\n"
                 "movq %cr0, %rbx\n"
                 "movq %rbx, 0x90(%rax)\n"
                 "movq %cr2, %rbx\n"
                 "movq %rbx, 0x98(%rax)\n"
                 "movq %cr3, %rbx\n"
                 "movq %rbx, 0xa0(%rax)\n"
                 "popq %rbx\n"
                 "popq %rax\n"
                 "retq\n");
}

static void print_registers(const struct debug_registers *r) {
    kprintf("rip: %#018lx rflags: %#018lx\n", r->rip, r->rflags);
    kprintf("rdi: %#018lx rsi: %#018lx rbp: %#018lx\n", r->rdi, r->rsi, r->rbp);
    kprintf("rsp: %#018lx rbx: %#018lx rdx: %#018lx\n", r->rsp, r->rbx, r->rdx);
    kprintf("rcx: %#018lx rax: %#018lx r8:  %#018lx\n", r->rcx, r->rax, r->r8);
    kprintf("r9:  %#018lx r10: %#018lx r11: %#018lx\n", r->r9, r->r10, r->r11);
    kprintf("r12: %#018lx r13: %#018lx r14: %#018lx\n", r->r12, r->r13, r->r14);
    kprintf("r15: %#018lx\n", r->r15);
    kprintf("cr0: %#018lx cr2: %#018lx cr3: %#018lx\n", r->cr0, r->cr2, r->cr3);
}

void print_registers_state(const struct registers_state *r) {
    kprintf("rip: %#018lx rflags: %#018lx\n", r->rip, r->rflags);
    kprintf("rdi: %#018lx rsi: %#018lx rbp: %#018lx\n", r->rdi, r->rsi, r->rbp);
    kprintf("rsp: %#018lx rbx: %#018lx rdx: %#018lx\n", r->rsp, r->rbx, r->rdx);
    kprintf("rcx: %#018lx rax: %#018lx r8:  %#018lx\n", r->rcx, r->rax, r->r8);
    kprintf("r9:  %#018lx r10: %#018lx r11: %#018lx\n", r->r9, r->r10, r->r11);
    kprintf("r12: %#018lx r13: %#018lx r14: %#018lx\n", r->r12, r->r13, r->r14);
    kprintf("r15: %#018lx\n", r->r15);
}

void dump_registers(void) {
    struct debug_registers regs;
    get_registers(&regs);
    print_registers(&regs);
}

void init_process_registers(struct registers_state *regs, void (*fn)(void *),
                            void *arg, u64 stack_end) {
    regs->rip = (u64)fn;
    regs->rflags = read_cpu_flags() & ~X86_FLAGS_IF;
    regs->rsp = stack_end;
    regs->rbp = stack_end;
    regs->rdi = (u64)arg;
}

__naked __noinline void switch_process(struct process *from __unused,
                                       struct process *to __unused) {
    // here we explicitly clobber all the registers to save us from saving them
    asm volatile("movq %%rsp, %P[rsp_off](%%rdi)\n"
                 "leaq 1f(%%rip), %%rax\n"
                 "movq %%rax, %P[rip_off](%%rdi)\n"

                 "movq %P[rsp_off](%%rsi), %%rsp\n"
                 "pushq %P[rip_off](%%rsi)\n"
                 "jmp switch_to\n"

                 "1:\n"
                 "retq\n"

                 ::[rsp_off] "i"(offsetof(struct process, execution_state.rsp)),
                 [rip_off] "i"(offsetof(struct process, execution_state.rip))
                 : "memory", "cc", "rcx", "rbx", "rdx", "r8", "r9", "r10",
                   "r11", "r12", "r13", "r14", "r15");
}

void init_percpu(void) {
    // TODO: This is certainly not nice
    extern struct process idle_process;
    struct percpu *percpu = kzalloc(sizeof(*percpu));
    expects(percpu);
    percpu->this = percpu;
    percpu->current = &idle_process;
    write_msr(MSR_GS_BASE, (u64)percpu);
}
