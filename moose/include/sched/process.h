#pragma once

#include <bitops.h>
#include <list.h>
#include <sched/locks.h>
#include <types.h>

#define MAX_PROCESSES 256
#define PROCESS_MAX_FILES 256

struct address_space {};

enum process_state {
    PROCESS_RUNNING,
    PROCESS_SLEEPING,
    PROCESS_DEAD
};

struct process {
    const char *name;

    enum process_state state;
    pid_t pid;
    pid_t ppid;
    mode_t umask;
    void *saved_execution_state;

    struct address_space as;
    struct file *files[PROCESS_MAX_FILES];
    spinlock_t lock;

    struct list_head list;
};

struct scheduler {
    bitmap_t pid_bitmap[BITS_TO_BITMAP(MAX_PROCESSES)];

    atomic_t preempt_count;
    struct process *current;
    struct list_head process_list;
    spinlock_t lock;
};

struct process *get_current(void);
void init_scheduler(void);
void launch_process(const char *name, void (*function)(void *), void *arg);
void switch_process(struct process *from, struct process *to);
void schedule(void);
void yield(void);
void preempt_disable(void);
void preempt_enable(void);
int get_preempt_count(void);

