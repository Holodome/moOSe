#include <arch/cpu.h>
#include <kthread.h>
#include <mm/kmalloc.h>
#include <sched/locks.h>

extern void bootstrap_task(u64 rsp, u64 rip);

LIST_HEAD(tasks);
static int pid_counter = 1;
struct task current_proxy = {.pid = 0};
volatile struct task *current = &current_proxy;

static struct task *create_task(void (*fn)(void)) {
    struct task *task = kzalloc(sizeof(*task) + sizeof(union kthread));
    if (task == NULL)
        return NULL;

    task->info = (void *)(task + 1);
    u64 stack_end = (u64)task->info + sizeof(union kthread);
    task->regs.rip = (u64)fn;
    task->regs.cs = 8;
    task->regs.rflags = read_cpu_flags();
    task->regs.ursp = stack_end;
    task->regs.uss = 16;
    task->pid = pid_counter++;

    return task;
}

int launch_first_task(void (*fn)(void)) {
    struct task *task = create_task(fn);
    if (task == NULL)
        return -1;

    task->name = "init";
    current = task;
    list_add(&task->list, &tasks);
    bootstrap_task(current->regs.ursp, current->regs.rip);
    return 0;
}

int launch_task(const char *name, void (*fn)(void)) {
    struct task *task = create_task(fn);
    if (task == NULL)
        return -1;

    task->name = name;
    list_add(&task->list, &tasks);
    return 0;
}
