#include <arch/jiffies.h>
#include <assert.h>
#include <kstdio.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <param.h>
#include <sched/sched.h>
#include <string.h>

struct process idle_process = {
    .name = "idle", .umask = 0666, .lock = INIT_SPIN_LOCK()};
static struct scheduler scheduler_ = {
    .lock = INIT_SPIN_LOCK(),
    .process_list = INIT_LIST_HEAD(scheduler_.process_list)};
static struct scheduler *__scheduler = &scheduler_;

static pid_t alloc_pid(struct scheduler *scheduler) {
    u64 bit = bitmap_first_set(scheduler->pid_bitmap, MAX_PROCESSES);
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
    idle_process.sched.nice = DEFAULT_NICE;
    idle_process.sched.prio = nice_to_prio(idle_process.sched.nice);
    idle_process.state = PROCESS_RUNNING;
    list_add_tail(&idle_process.sched_list,
                  __scheduler->rq.ranks + idle_process.sched.prio);
    list_add(&idle_process.list, &__scheduler->process_list);
}

static void init_rq(void) {
    for (size_t i = 0; i < ARRAY_SIZE(__scheduler->rq.ranks); ++i)
        init_list_head(__scheduler->rq.ranks + i);
}

void init_scheduler(void) {
    init_rq();
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
    process->sched.nice = DEFAULT_NICE;
    process->sched.timeslice = DEFAULT_TIMESLICE;
    process->sched.prio = nice_to_prio(process->sched.nice);
    process->state = PROCESS_INTERRUPTIBLE;

    cpuflags_t flags = spin_lock_irqsave(&__scheduler->lock);
    list_add_tail(&process->sched_list,
                  __scheduler->rq.ranks + process->sched.prio);
    set_bit(process->sched.prio, __scheduler->rq.bitmap);
    list_add(&process->list, &__scheduler->process_list);
    spin_unlock_irqrestore(&__scheduler->lock, flags);
}

static void context_switch(struct process *from, struct process *to) {
    switch_process(from, to);
}

static int update_current(struct process *current) {
    expects(current->state == PROCESS_RUNNING);
    struct process_sched_info *si = &current->sched;
    u64 jiffies = get_jiffies();
    u64 expired = jiffies - si->timeslice_start_jiffies;
    if (expired > si->timeslice) {
        u32 old_prio = si->prio;
        if (__unlikely(si->prio == MAX_PRIO)) {
            si->prio = nice_to_prio(si->nice);
            ++si->timeslice;
        } else {
            ++si->prio;
        }

        list_remove(&current->sched_list);
        if (list_is_empty(__scheduler->rq.ranks + old_prio))
            clear_bit(old_prio, __scheduler->rq.bitmap);
        list_add_tail(&current->sched_list, __scheduler->rq.ranks + si->prio);
        set_bit(si->prio, __scheduler->rq.bitmap);

        return 1;
    }

    return 0;
}

static struct process *pick_next_process(void) {
    u32 first_set = bitmap_first_set(__scheduler->rq.bitmap, MAX_PRIO);
    expects(first_set);
    --first_set;

    for (; first_set <= MAX_PRIO; ++first_set) {
        struct list_head *rank = __scheduler->rq.ranks + first_set;
        struct process *candidate;
        list_for_each_entry(candidate, rank, list) {
            if (candidate->state == PROCESS_INTERRUPTIBLE) {
                return candidate;
            } else if (candidate->state == PROCESS_UNINTERRUPTIBLE) {
                expects(!"todo");
            }
        }
    }

    return get_current();
}

void schedule(void) {
    struct process *current = get_current();

    if (!update_current(current))
        return;

    cpuflags_t flags = spin_lock_irqsave(&__scheduler->lock);
    struct process *new_process = pick_next_process();
    spin_unlock_irqrestore(&__scheduler->lock, flags);
    if (current != new_process)
        context_switch(current, new_process);
}

// called from switch_process to finalize switching after stack and pc
// have been changed
void switch_to(struct process *from, struct process *to) {
    from->state = PROCESS_INTERRUPTIBLE;
    to->state = PROCESS_RUNNING;
    set_current(to);
    to->sched.timeslice_start_jiffies = get_jiffies();
}
