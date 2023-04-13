#include <assert.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <sched/process.h>

static struct scheduler scheduler_ = {
    .process_list = INIT_LIST_HEAD(scheduler_.process_list)};
static struct scheduler *__scheduler = &scheduler_;

__used static struct file *get_file(struct process *p, int fd) {
    return fd < ARRAY_SIZE(p->files) ? p->files[fd] : NULL;
}

__used static int allocate_fd(struct process *p) {
    int result = -1;
    for (size_t i = 0; i < ARRAY_SIZE(p->files) && result == -1; ++i) {
        struct file *test = p->files[i];
        if (test == NULL)
            result = i;
    }
    return result;
}

static pid_t alloc_pid(struct scheduler *scheduler) {
    u64 bit = bitmap_first_clear(scheduler->pid_bitmap, MAX_PROCESSES);
    if (!bit)
        panic("Max count of processes reached");

    --bit;
    set_bit(bit, scheduler->pid_bitmap);

    return bit;
}

__used static void free_pid(struct scheduler *scheduler, pid_t pid) {
    expects(pid < MAX_PROCESSES);
    expects(test_bit(pid, scheduler->pid_bitmap));
    clear_bit(pid, scheduler->pid_bitmap);
}

void init_scheduler(void) {
}

void launch_process(const char *name, void (*function)(void *), void *arg) {
    struct process *process = kzalloc(sizeof(*process));
    expects(process);
    process->name = kstrdup(name);
    expects(process->name);
    init_process_registers(&process->execution_state, function, arg,
                           (u64)((&process->stack) + 1));
    process->stack.info.p = process;
    process->pid = alloc_pid(__scheduler);

    cpuflags_t flags = spin_lock_irqsave(&__scheduler->lock);
    list_add(&process->list, &__scheduler->process_list);
    spin_unlock_irqrestore(&__scheduler->lock, flags);
}

void schedule(void) {
    if (get_preempt_count())
        return;

    preempt_disable();

    preempt_enable();
}

void preempt_disable(void) {
    atomic_inc(&__scheduler->preempt_count);
}

void preempt_enable(void) {
    expects(get_preempt_count());
    atomic_dec(&__scheduler->preempt_count);
}

int get_preempt_count(void) {
    return atomic_read(&__scheduler->preempt_count);
}

void switch_to(void) {
}

void yield(void) {
    schedule();
}

struct process *get_current(void) {
    return __scheduler->current;
}
