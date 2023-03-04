#pragma once

#include <arch/amd64/cpu.h>
#include <types.h>

void dump_registers(void);

void set_stack(u64 sp, u64 old_stack_base);
