#include <assert.h>
#include <kstdio.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <param.h>
#include <sched/process.h>
#include <sched/scheduler.h>
#include <string.h>

struct process idle_process = {
    .name = "idle", .umask = 0666, .lock = INIT_SPIN_LOCK()};
static struct scheduler scheduler_ = {
    .lock = INIT_SPIN_LOCK(),
    .process_list = INIT_LIST_HEAD(scheduler_.process_list)};
static struct scheduler *__scheduler = &scheduler_;

static pid_t alloc_pid(struct scheduler *scheduler) {
    u64 bit = bitmap_first_clear(scheduler->pid_bitmap, MAX_PROCESSES);
    if (!bit)
        panic("max count of processes reached");

    --bit;
    clear_bit(bit, scheduler->pid_bitmap);

    return bit;
}

__used static void free_pid(struct scheduler *scheduler, pid_t pid) {
    expects(pid < MAX_PROCESSES);
    expects(test_bit(pid, scheduler->pid_bitmap));
    clear_bit(pid, scheduler->pid_bitmap);
}

static void init_idle_stack(void) {
    u64 stack_base_address =
        FIXUP_ADDR(KERNEL_INITIAL_STACK - sizeof(union process_stack));
    idle_process.stack = (void *)stack_base_address;
    idle_process.stack->info.p = &idle_process;
    list_add(&idle_process.list, &__scheduler->process_list);
}

void init_scheduler(void) {
    memset(__scheduler->pid_bitmap, 0xff, sizeof(__scheduler->pid_bitmap));
    clear_bit(0, __scheduler->pid_bitmap);
    init_idle_stack();
}

void launch_process(const char *name, void (*function)(void *), void *arg) {
    struct process *process = kzalloc(sizeof(*process));
    expects(process);
    process->stack = kzalloc(sizeof(*process->stack));
    expects(process->stack);
    process->name = kstrdup(name);
    expects(process->name);
    init_process_registers(&process->execution_state, function, arg,
                           (u64)((process->stack) + 1));
    process->stack->info.p = process;
    process->pid = alloc_pid(__scheduler);

    cpuflags_t flags = spin_lock_irqsave(&__scheduler->lock);
    list_add(&process->list, &__scheduler->process_list);
    spin_unlock_irqrestore(&__scheduler->lock, flags);
}

static void context_switch(struct process *from, struct process *to) {
    switch_process(from, to);
}

void schedule(void) {
    if (get_preempt_count())
        return;

    cpuflags_t flags = spin_lock_irqsave(&__scheduler->lock);
    struct process *current = get_current();
    struct process *next =
        list_next_entry_or_null(current, &__scheduler->process_list, list);
    if (!next)
        next =
            list_first_entry(&__scheduler->process_list, struct process, list);

    spin_unlock_irqrestore(&__scheduler->lock, flags);
    if (next != current)
        context_switch(current, next);
}

// called from switch_process to finalize switching after stack and pc
// have been changed
void switch_to(struct process *from, struct process *to) {
    set_current(to);
    (void)from;
}

