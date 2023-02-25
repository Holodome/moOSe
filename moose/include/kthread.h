#pragma once

#include <types.h>

#define KTHREAD_STACK_SIZE 4096

struct task_cpu_state {
    u64 rflags;
    u64 rip;
    u64 rsp;
};

struct task {
    //
};

struct kthread_info {
    struct task *task;
};

union kthread {
    struct kthread_info info;
    u64 stack[KTHREAD_STACK_SIZE / sizeof(u64)];
};

int init_kinit_thread(void);

void process_switch(struct task *old, struct task *new);
