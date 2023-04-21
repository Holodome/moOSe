#pragma once

#include <arch/cpu.h>
#include <bitops.h>
#include <list.h>
#include <sched/locks.h>

#define MAX_PROCESSES 256
#define PROCESS_MAX_FILES 256
#define PROCESS_STACK_SIZE (4096 * 4)

enum process_state {
    PROCESS_RUNNING,
    PROCESS_SLEEPING,
    PROCESS_DEAD
};

struct process_info {
    struct process *p;
};

union process_stack {
    struct process_info info;
    u64 stack[PROCESS_STACK_SIZE / sizeof(u64)];
};

struct process {
    struct registers_state execution_state;
    const char *name;

    enum process_state state;
    pid_t pid;
    pid_t ppid;
    mode_t umask;

    struct file *files[PROCESS_MAX_FILES];
    spinlock_t lock;

    struct list_head list;

    union process_stack *stack;
};

void exit_current(void);
