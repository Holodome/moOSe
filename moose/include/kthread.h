#pragma once

#include <types.h>

#define KTHREAD_STACK_SIZE 4096

union kthread {
    u64 stack[KTHREAD_STACK_SIZE / sizeof(u64)];
};

int init_kinit_thread(void);
