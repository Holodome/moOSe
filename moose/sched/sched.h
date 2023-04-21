#pragma once

#include <bitops.h>
#include <list.h>
#include <rbtree.h>
#include <sched/locks.h>

#define MAX_PROCESSES 256
#define PROCESS_MAX_FILES 256
#define PROCESS_STACK_SIZE (4096 * 4)

#define MAX_NICE 19
#define MIN_NICE -20
#define MAX_PRIO 40

#define prio_to_nice(_prio) (-(int)(_prio) + 20)
#define nice_to_prio(_nice) (-(int)(_nice) + 20)

struct runqueue {
    bitmap_t bitmap[BITS_TO_BITMAP(MAX_PRIO)];
    struct list_head ranks[MAX_PRIO];
};

struct scheduler {
    bitmap_t pid_bitmap[BITS_TO_BITMAP(MAX_PROCESSES)];
    struct runqueue rq;

    struct list_head process_list;
    spinlock_t lock;
};

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

void init_scheduler(void);
void launch_process(const char *name, void (*function)(void *), void *arg);
void switch_process(struct process *from, struct process *to);
void schedule(void);
void exit_current(void);
