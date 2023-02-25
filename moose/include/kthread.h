#pragma once

#include <types.h>

#define KTHREAD_STACK_SIZE 4096

struct task {
    void *stack;
    void *eip;

    struct kthread_info *info;
    struct list_head next;
};

struct kthread_info {
    struct task *task;
};

union kthread {
    struct kthread_info info;
    u64 stack[KTHREAD_STACK_SIZE / sizeof(u64)];
};

int init_kinit_thread(void);

void schedule(void);
void process_switch(struct task *old, struct task *new);
void create_process(void (*fn)(void *), void *args);

extern struct task *current;
