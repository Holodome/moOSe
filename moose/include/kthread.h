#pragma once

#include <list.h>
#include <types.h>
// FIXME: Cleanup this include
#include <arch/amd64/idt.h>

#define KTHREAD_STACK_SIZE 4096

struct task {
    struct registers_state regs;
    struct kthread_info *info;
    struct list_head list;
};

struct kthread_info {
    struct task *task;
};

union kthread {
    struct kthread_info info;
    u64 stack[KTHREAD_STACK_SIZE / sizeof(u64)];
};

int init_kinit_thread(void (*fn)(void));
void launch_thread(void (*fn)(void));

extern volatile struct task *current;
extern struct list_head tasks;
