#pragma once

#include <types.h>

struct idt_entry {
    u16 offset_low;
    u16 selector;
    u8 ist;
    u8 attr;
    u16 offset_mid;
    u32 offset_high;
    u32 reserved;
} __attribute__((packed));

struct idt_reg {
    u64 offset;
    u16 size;
} __attribute__((packed));

struct registers_state {
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rsp;
    u64 rbx;
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

    u16 exception_code;
    u16 isr_number;

    u64 rip;
    u64 cs;
    u64 rflags;
};

typedef void isr_t(const struct registers_state *regs);

void isr_handler(const struct registers_state *regs);
void setup_idt(void);
