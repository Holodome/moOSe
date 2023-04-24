#include <moose/arch/amd64/asm.h>
#include <moose/arch/amd64/cpuid.h>
#include <moose/arch/cpu.h>
#include <moose/assert.h>
#include <moose/kstdio.h>
#include <moose/mm/kmalloc.h>
#include <moose/param.h>
#include <moose/sched/sched.h>
#include <moose/sys/syscalls.h>

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

struct gdt_entry {
    union {
        struct {
            u32 low;
            u32 high;
        };
        struct {
            u16 limit_lo;
            u16 base_lo;
            u8 base_hi;
            u8 type : 4;
            u8 descriptor_type : 1;
            u8 dpl : 2;
            u8 segment_present : 1;
            u8 limit_hi : 4;
            u8 : 1;
            u8 operation_size64 : 1;
            u8 operation_size32 : 1;
            u8 granularity : 1;
            u8 base_hi2;
        };
    };
};

static_assert(sizeof(struct gdt_entry) == 8);

struct gdt_reg {
    u16 size;
    u64 offset;
} __packed;

static struct gdt_entry gdt[256] __aligned(16);
static struct gdt_reg gdtr;

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

__naked __noinline void syscall_entry(void) {
    asm volatile(
        "movq %%rsp, %%gs:%c[user_stack]\n"
        "movq %%gs:%c[kernel_stack], %%rsp\n"
        // build registers_state
        "pushq $0x1b\n"               // uss
        "pushq %%gs:%c[user_stack]\n" // ursp
        "sti\n"
        "pushq %%r11\n" // rflags
        "pushq $0x23\n" // cs
        "pushq %%rcx\n" // rip
        "pushq $0x0\n"  // exception_code
        "pushq $0x0\n"  // isr_number
        "pushq %%r15\n"
        "pushq %%r14\n"
        "pushq %%r13\n"
        "pushq %%r12\n"
        "pushq %%r11\n"
        "pushq %%r10\n"
        "pushq %%r9\n"
        "pushq %%r8\n"
        "pushq %%rax\n"
        "pushq %%rcx\n"
        "pushq %%rdx\n"
        "pushq %%rbx\n"
        "pushq %%rsp\n"
        "pushq %%rbp\n"
        "pushq %%rsi\n"
        "pushq %%rdi\n"

        "movq %%rsp, %%rdi\n"
        "call syscall_handler\n"

        "popq %%rdi\n"
        "popq %%rsi\n"
        "popq %%rbp\n"
        "addq $8, %%rsp\n"
        "popq %%rbx\n"
        "popq %%rdx\n"
        "popq %%rcx\n"
        "popq %%rax\n"
        "popq %%r8\n"
        "popq %%r9\n"
        "popq %%r10\n"
        "popq %%r11\n"
        "popq %%r12\n"
        "popq %%r13\n"
        "popq %%r14\n"
        "popq %%r15\n"
        "addq $16, %%rsp\n"
        "popq %%rcx\n"
        "addq $0x10, %%rsp\n"
        "cli\n"
        "popq %%rsp\n"
        "sysretq\n" ::[user_stack] "i"(offsetof(struct percpu, user_stack)),
        [kernel_stack] "i"(offsetof(struct percpu, kernel_stack)));
}

void parse_syscall_parameters(const struct registers_state *state,
                              struct syscall_parameters *params) {
    params->function = state->rax;
    params->arg0 = state->rdi;
    params->arg1 = state->rsi;
    params->arg2 = state->rdx;
    params->arg3 = state->r10;
    params->arg4 = state->r8;
}

void set_syscall_result(u64 result, struct registers_state *state) {
    state->rax = result;
}

static void init_percpu(void) {
    // TODO: This is certainly not nice
    extern struct process idle_process;
    struct percpu *percpu = kzalloc(sizeof(*percpu));
    expects(percpu);
    percpu->this = percpu;
    percpu->current = &idle_process;
    write_msr(MSR_GS_BASE, (u64)percpu);
}

static void setup_syscall(void) {
    if (!cpu_supports(CPUID_SYSCALL))
        panic("cpu must suport syscall instruction");
    write_msr(MSR_EFER, read_msr(MSR_EFER) | 0x1);

    u64 star = (0x13ul << 48u) | (0x08ul << 32u);
    write_msr(MSR_STAR, star);
    write_msr(MSR_LSTAR, (u64)syscall_entry);
    write_msr(MSR_SFMASK, 0x257fd5u);

    struct percpu *percpu = get_percpu();
    percpu->kernel_stack =
        (u64)kmalloc(sizeof(union process_stack)) + sizeof(union process_stack);
}

static void init_gdt(void) {
    gdt[0].low = 0x00000000;
    gdt[0].high = 0x00000000;
    gdt[KERNEL_CS >> 3].low = 0x0000ffff;
    gdt[KERNEL_CS >> 3].high = 0x00af9a00;
    gdt[KERNEL_DS >> 3].low = 0x0000ffff;
    gdt[KERNEL_DS >> 3].high = 0x00af9200;
#if 0
    gdt[USER_DS >> 3].low = 0x0000ffff;
    gdt[USER_DS >> 3].high = 0x008ff200;
    gdt[USER_CS >> 3].low = 0x0000ffff;
    gdt[USER_CS >> 3].high = 0x00affa00;
#else
    gdt[USER_DS >> 3].low = 0x0000ffff;
    gdt[USER_DS >> 3].high = 0x00af9200;
    gdt[USER_CS >> 3].low = 0x0000ffff;
    gdt[USER_CS >> 3].high = 0x00af9a00;
#endif

    gdtr.offset = (u64)&gdt;
    gdtr.size = sizeof(gdt) - 1;
    asm volatile("lidt %0\n" : : "m"(gdtr));
}

void init_cpu(void) {
    init_percpu();
    init_cpuid();
    init_gdt();
    setup_syscall();
}
