#include "idt.h"

#include "kstdio.h"

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
    static struct idt_reg idt_reg;
    idt_reg.offset = (u64)&idt[0];
    idt_reg.size = sizeof(idt) - 1;
    asm volatile("lidt %0" : : "m"(idt_reg));
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
    if (exception < ARRAY_SIZE(strs))
        result = strs[exception];

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

void exception_handler(const struct registers_state *regs) {
    unsigned no = regs->isr_number;
    if (no < 32) {
        kprintf("exception %s(%u)\n", get_exception_name(no), no);
    } else {
        isr_t *isr = isrs[no];
        if (isr != NULL)
            isr(regs);
    }
}
