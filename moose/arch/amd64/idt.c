#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <kstdio.h>

// Port address for master PIC
#define PIC1 0x20
// Port address for slave PIC
#define PIC2 0xA0
#define PIC1_CMD PIC1
#define PIC1_DAT (PIC1 + 1)
#define PIC2_CMD PIC2
#define PIC2_DAT (PIC2 + 1)

#define PIC_EOI 0x20 /* End-of-interrupt command code */
#define IRQ_BASE 32

#define EXCEPTION_PAGE_FAULT 0xe

static struct idt_entry idt[256] __attribute__((aligned(16)));
static isr_t *isrs[48];

static void set_idt_entry(u8 n, u64 isr_addr) {
    struct idt_entry *e = idt + n;
    e->offset_low = isr_addr & 0xffff;
    e->selector = 0x08;
    e->ist = 0;
    e->attr = 0x8e;
    e->offset_mid = (isr_addr >> 16) & 0xffff;
    e->offset_high = isr_addr >> 32;
    e->reserved = 0;
}

static void load_idt(void) {
    struct idt_reg idt_reg;
    idt_reg.offset = (u64)&idt[0];
    idt_reg.size = 256 * sizeof(struct idt_entry) - 1;
    asm volatile("lidt %0\n" : : "m"(idt_reg));
}

__attribute__((used)) static void
print_registers(const struct registers_state *r) {
    kprintf("exception_code: %u, isr: %u\n", (unsigned)r->exception_code,
            (unsigned)r->isr_number);
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
    kprintf("rip: %#018llx cs:  %#018llx rflags: %#018llx\n", r->rip, r->cs,
            r->rflags);
    kprintf("ursp: %#018llx uss: %#018llx\n", r->ursp, r->uss);
}

static const char *get_exception_name(unsigned exception) {
    static const char *strs[] = {
        "division by zero",
        "debug",
        "nmi",
        "breakpoint",
        "into overflow",
        "out of bounds",
        "invalid opcode",
        "no coprocessor",
        "double fault",
        "coprocessor segment overrun",
        "bad tss",
        "segment not present",
        "stack fault",
        "general protection fault",
        "page fault",
        "unknown interrupt",
        "coprocessor fault",
        "alignment check",
        "machine check",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
    };
    static_assert(ARRAY_SIZE(strs) == 32);

    const char *result = NULL;
    if (exception < ARRAY_SIZE(strs)) result = strs[exception];

    return result;
}

extern void exception0();
extern void exception1();
extern void exception2();
extern void exception3();
extern void exception4();
extern void exception5();
extern void exception6();
extern void exception7();
extern void exception8();
extern void exception9();
extern void exception10();
extern void exception11();
extern void exception12();
extern void exception13();
extern void exception14();
extern void exception15();
extern void exception16();
extern void exception17();
extern void exception18();
extern void exception19();
extern void exception20();
extern void exception21();
extern void exception22();
extern void exception23();
extern void exception24();
extern void exception25();
extern void exception26();
extern void exception27();
extern void exception28();
extern void exception29();
extern void exception30();
extern void exception31();

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();

static void eoi(u8 irq) {
    if (irq >= 8 + IRQ_BASE) port_out8(PIC2_CMD, PIC_EOI);

    port_out8(PIC1_CMD, PIC_EOI);
}

void isr_handler(struct registers_state *regs) {
    unsigned no = regs->isr_number;
    if (no < 32) {
        kprintf("exception %s(%u): %u\n", get_exception_name(no), no,
                (unsigned)regs->exception_code);
        if (no == EXCEPTION_PAGE_FAULT) {
            kprintf("address: %#018llx\n", read_cr2());
        }
        print_registers(regs);
        cli();
        hlt();
    } else {
        isr_t *isr = isrs[no];
        if (isr != NULL) isr(regs);
    }

    eoi(no);
}

void register_isr(int num, isr_t *isr) { isrs[IRQ_BASE + num] = isr; }

void init_idt(void) {
    set_idt_entry(0, (u64)exception0);
    set_idt_entry(1, (u64)exception1);
    set_idt_entry(2, (u64)exception2);
    set_idt_entry(3, (u64)exception3);
    set_idt_entry(4, (u64)exception4);
    set_idt_entry(5, (u64)exception5);
    set_idt_entry(6, (u64)exception6);
    set_idt_entry(7, (u64)exception7);
    set_idt_entry(8, (u64)exception8);
    set_idt_entry(9, (u64)exception9);
    set_idt_entry(10, (u64)exception10);
    set_idt_entry(11, (u64)exception11);
    set_idt_entry(12, (u64)exception12);
    set_idt_entry(13, (u64)exception13);
    set_idt_entry(14, (u64)exception14);
    set_idt_entry(15, (u64)exception15);
    set_idt_entry(16, (u64)exception16);
    set_idt_entry(17, (u64)exception17);
    set_idt_entry(18, (u64)exception18);
    set_idt_entry(19, (u64)exception19);
    set_idt_entry(20, (u64)exception20);
    set_idt_entry(21, (u64)exception21);
    set_idt_entry(22, (u64)exception22);
    set_idt_entry(23, (u64)exception23);
    set_idt_entry(24, (u64)exception24);
    set_idt_entry(25, (u64)exception25);
    set_idt_entry(26, (u64)exception26);
    set_idt_entry(27, (u64)exception27);
    set_idt_entry(28, (u64)exception28);
    set_idt_entry(29, (u64)exception29);
    set_idt_entry(30, (u64)exception30);
    set_idt_entry(31, (u64)exception31);

    // Remap the PIC
    port_out8(0x20, 0x11);
    port_out8(0xA0, 0x11);
    port_out8(0x21, 0x20);
    port_out8(0xA1, 0x28);
    port_out8(0x21, 0x04);
    port_out8(0xA1, 0x02);
    port_out8(0x21, 0x01);
    port_out8(0xA1, 0x01);
    port_out8(0x21, 0x0);
    port_out8(0xA1, 0x0);

    // Install the IRQs
    set_idt_entry(32, (u64)isr0);
    set_idt_entry(33, (u64)isr1);
    set_idt_entry(34, (u64)isr2);
    set_idt_entry(35, (u64)isr3);
    set_idt_entry(36, (u64)isr4);
    set_idt_entry(37, (u64)isr5);
    set_idt_entry(38, (u64)isr6);
    set_idt_entry(39, (u64)isr7);
    set_idt_entry(40, (u64)isr8);
    set_idt_entry(41, (u64)isr9);
    set_idt_entry(42, (u64)isr10);
    set_idt_entry(43, (u64)isr11);
    set_idt_entry(44, (u64)isr12);
    set_idt_entry(45, (u64)isr13);
    set_idt_entry(46, (u64)isr14);
    set_idt_entry(47, (u64)isr15);

    load_idt();
    sti();
}

