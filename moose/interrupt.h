#pragma once

#include "types.h"

struct __attribute__((packed)) idt_gate {
    u16 isr_low;
    u16 cs;
    u8 reserved;
    u8 flags;
    u16 isr_high;
} __attribute__((aligned(16)));

struct __attribute__((packed)) idt_reg {
    u16 limit;
    u32 base;
};

struct isr_regs {
    u32 ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, useresp, ss;
};

typedef void (*isr_t)(struct isr_regs *);

__attribute__((noreturn)) void exception_handler(void);
void register_irq_handler(u8 n, isr_t handler);

void irq_handler(struct isr_regs *regs);
void isr_handler(struct isr_regs *regs);

void init_interrupts(void);
