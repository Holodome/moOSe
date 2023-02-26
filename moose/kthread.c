#include <arch/cpu.h>
#include <assert.h>
#include <kstdio.h>
#include <kthread.h>
#include <mm/kmalloc.h>
#include <param.h>

extern void bootstrap_task(u64 rsp, u64 rip);

LIST_HEAD(tasks);
volatile struct task *current;

static struct task *create_task(void (*fn)(void)) {
    struct task *task = kzalloc(sizeof(*task));
    if (task == NULL) return NULL;

    task->info = kzalloc(sizeof(union kthread));
    if (task->info == NULL) {
        kfree(task);
        return NULL;
    }

    u64 stack_end = (u64)task->info + sizeof(union kthread);
    task->regs.rip = (u64)fn;
    task->regs.cs = 8;
    task->regs.rflags = read_cpu_flags();
    task->regs.ursp = stack_end;
    task->regs.uss = 16;

    return task;
}

int launch_first_task(void (*fn)(void)) {
    struct task *task = create_task(fn);
    if (task == NULL) return -1;

    current = task;
    list_add(&task->list, &tasks);
    bootstrap_task(current->regs.ursp, current->regs.rip);
    return 0;
}

int launch_task(void (*fn)(void)) {
    struct task *task = create_task(fn);
    if (task == NULL) return -1;

    list_add(&task->list, &tasks);
    return 0;
}

