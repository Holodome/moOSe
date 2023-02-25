#pragma once

#include <types.h>

void dump_registers(void);
__attribute__((noreturn)) void halt_cpu(void);

void set_stack(u64 sp, u64 old_stack_base);
