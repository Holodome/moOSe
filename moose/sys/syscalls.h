#pragma once

#include <types.h>

struct registers_state;

struct sysclass_parameters {
    u64 function;
    u64 arg0;
    u64 arg1;
    u64 arg2;
    u64 arg3;
    u64 arg4;
    u64 arg5;
};

void parse_sysclass_parameters(const struct registers_state *state,
                               struct sysclass_parameters *params);
void set_syscall_result(u64 result, struct registers_state *state);

void syscall_handler(struct registers_state *state);
