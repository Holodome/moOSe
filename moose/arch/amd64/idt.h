#pragma once

#include <moose/types.h>

struct registers_state;

struct idt_entry {
    u16 offset_low;
    u16 selector;
    u8 ist;
    u8 attr;
    u16 offset_mid;
    u32 offset_high;
    u32 reserved;
};

static_assert(sizeof(struct idt_entry) == 16);

struct idt_reg {
    u16 size;
    u64 offset;
} __packed;

void init_idt(void);
__noinline void eoi(u8 num);
