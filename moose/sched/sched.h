#pragma once

#include <bitops.h>
#include <list.h>
#include <rbtree.h>
#include <sched/locks.h>

#define MAX_PROCESSES 256
#define PROCESS_MAX_FILES 256
#define PROCESS_STACK_SIZE (4096 * 4)
#define DEFAULT_TIMESLICE 10

#define MAX_NICE 19
#define MIN_NICE -20
#define MAX_PRIO 39

#define prio_to_nice(_prio) ((int)(_prio)-20)
#define nice_to_prio(_nice) (u32)((int)(_nice) + 20)

struct process_sched_info {
    int nice;
    u32 timeslice;
    u32 prio;

    u64 current_time;
    u64 timeslice_start_jiffies;
};

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
    PROCESS_INTERRUPTIBLE,
    PROCESS_UNINTERRUPTIBLE,
    PROCESS_STOPPED,
    PROCESS_ZOMBIE
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
    struct process_sched_info sched;

    const char *name;

    enum process_state state;
    pid_t pid;
    pid_t ppid;
    mode_t umask;
    struct file *files[PROCESS_MAX_FILES];

    struct list_head list;
    struct list_head sched_list;

    union process_stack *stack;
    spinlock_t lock;
};

void init_scheduler(void);
void launch_process(const char *name, void (*function)(void *), void *arg);
void switch_process(struct process *from, struct process *to);
void schedule(void);
void exit_current(void);
