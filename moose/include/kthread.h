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

typedef void task_fn_t(void);

int launch_first_task(task_fn_t *fn);
int launch_task(task_fn_t *fn);

extern volatile struct task *current;
extern struct list_head tasks;

