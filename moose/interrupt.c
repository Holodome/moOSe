#include "interrupt.h"

#include "arch/ports.h"
#include "kprint.h"

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

static struct idt_gate idt[256];
static struct idt_reg idt_reg;

static isr_t isrs[256];

static void set_idt_gate(u8 n, u32 isr) {
    struct idt_gate *gate = idt + n;
    gate->isr_low = isr & 0xffff;
    gate->cs = 0x08;
    gate->reserved = 0;
    gate->flags = 0x8e;
    gate->isr_high = isr >> 16;
}

static void load_idt(void) {
    idt_reg.base = (u32)&idt;
    idt_reg.limit = 256 * sizeof(struct idt_gate) - 1;
    asm volatile("lidt (%0)" : : "r"(&idt_reg));
}

void exception_handler(void) {
    asm volatile("cli; hlt");
    __builtin_unreachable();
}

static void eoi(u8 irq) {
    if (irq >= 8)
        port_u8_out(PIC2_CMD, PIC_EOI);

    port_u8_out(PIC1_CMD, PIC_EOI);
}

void register_irq_handler(u8 n, isr_t handler) { isrs[n] = handler; }

void irq_handler(struct isr_regs *regs) {
    isr_t isr = isrs[regs->int_no];
    if (isr != NULL)
        isr(regs);

    eoi(regs->int_no - IRQ_BASE);
}

void isr_handler(struct isr_regs *regs) { kprintf("isr %x\n", regs->int_no); }

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
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void init_interrupts(void) {
    set_idt_gate(0, (u32)isr0);
    set_idt_gate(1, (u32)isr1);
    set_idt_gate(2, (u32)isr2);
    set_idt_gate(3, (u32)isr3);
    set_idt_gate(4, (u32)isr4);
    set_idt_gate(5, (u32)isr5);
    set_idt_gate(6, (u32)isr6);
    set_idt_gate(7, (u32)isr7);
    set_idt_gate(8, (u32)isr8);
    set_idt_gate(9, (u32)isr9);
    set_idt_gate(10, (u32)isr10);
    set_idt_gate(11, (u32)isr11);
    set_idt_gate(12, (u32)isr12);
    set_idt_gate(13, (u32)isr13);
    set_idt_gate(14, (u32)isr14);
    set_idt_gate(15, (u32)isr15);
    set_idt_gate(16, (u32)isr16);
    set_idt_gate(17, (u32)isr17);
    set_idt_gate(18, (u32)isr18);
    set_idt_gate(19, (u32)isr19);
    set_idt_gate(20, (u32)isr20);
    set_idt_gate(21, (u32)isr21);
    set_idt_gate(22, (u32)isr22);
    set_idt_gate(23, (u32)isr23);
    set_idt_gate(24, (u32)isr24);
    set_idt_gate(25, (u32)isr25);
    set_idt_gate(26, (u32)isr26);
    set_idt_gate(27, (u32)isr27);
    set_idt_gate(28, (u32)isr28);
    set_idt_gate(29, (u32)isr29);
    set_idt_gate(30, (u32)isr30);
    set_idt_gate(31, (u32)isr31);

    // Remap the PIC
    port_u8_out(0x20, 0x11);
    port_u8_out(0xA0, 0x11);
    port_u8_out(0x21, 0x20);
    port_u8_out(0xA1, 0x28);
    port_u8_out(0x21, 0x04);
    port_u8_out(0xA1, 0x02);
    port_u8_out(0x21, 0x01);
    port_u8_out(0xA1, 0x01);
    port_u8_out(0x21, 0x0);
    port_u8_out(0xA1, 0x0);

    // Install the IRQs
    set_idt_gate(32, (u32)irq0);
    set_idt_gate(33, (u32)irq1);
    set_idt_gate(34, (u32)irq2);
    set_idt_gate(35, (u32)irq3);
    set_idt_gate(36, (u32)irq4);
    set_idt_gate(37, (u32)irq5);
    set_idt_gate(38, (u32)irq6);
    set_idt_gate(39, (u32)irq7);
    set_idt_gate(40, (u32)irq8);
    set_idt_gate(41, (u32)irq9);
    set_idt_gate(42, (u32)irq10);
    set_idt_gate(43, (u32)irq11);
    set_idt_gate(44, (u32)irq12);
    set_idt_gate(45, (u32)irq13);
    set_idt_gate(46, (u32)irq14);
    set_idt_gate(47, (u32)irq15);

    load_idt(); // Load with ASM
}
