#include <arch/cpu.h>
#include <assert.h>
#include <param.h>
#include <kmalloc.h>
#include <kstdio.h>
#include <kthread.h>

LIST_HEAD(tasks);
volatile struct task *current;

static struct task *create_task(void (*fn)(void)) {
    struct task *task = kzalloc(sizeof(*task));
    if (task == NULL)
        return NULL;

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

int init_kinit_thread(void (*fn)(void)) {
    struct task *task = create_task(fn);
    if (task == NULL)
        return -1;

    current = task;
    list_add(&task->list, &tasks);
    extern void bootstrap_task();
    bootstrap_task(current->regs.ursp, current->regs.rip);
    return 0;
}

void launch_thread(void (*fn)(void)) {
    struct task *task = create_task(fn);
    list_add(&task->list, &tasks);
}

