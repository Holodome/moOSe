#pragma once

#include <types.h>

struct isr_context;

struct idt_entry {
    u16 offset_low;
    u16 selector;
    u8 ist;
    u8 attr;
    u16 offset_mid;
    u32 offset_high;
    u32 reserved;
} __packed;

static_assert(sizeof(struct idt_entry) == 16);

struct idt_reg {
    u16 size;
    u64 offset;
} __packed;

typedef void isr_t(struct isr_context *regs);

void init_idt(void);
void register_isr(int num, isr_t *isr);
