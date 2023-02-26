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

static_assert(sizeof(struct idt_entry) == 16);

struct idt_reg {
    u16 size;
    u64 offset;
} __attribute__((packed));

struct registers_state {
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

    u64 isr_number;
    u64 exception_code;

    u64 rip;
    u64 cs;
    u64 rflags;
    u64 ursp;
    u64 uss;
} __attribute__((packed));

static_assert(sizeof(struct registers_state) == 184);

typedef void isr_t(struct registers_state *regs);

void isr_handler(struct registers_state *regs);
void init_idt(void);
void register_isr(int num, isr_t *isr);
