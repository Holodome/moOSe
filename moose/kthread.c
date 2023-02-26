#include <arch/cpu.h>
#include <kernel.h>
#include <kmalloc.h>
#include <kstdio.h>
#include <kthread.h>

static LIST_HEAD(tasks);
volatile struct task *current;

int init_kinit_thread(void (*fn)(void)) {
    struct task *task = create_task(fn);
    if (task == NULL)
        return -1;

    current = task;
    bootstrap_task(task);
    return 0;
}

struct task *create_task(void (*fn)(void)) {
    struct task *task = kzalloc(sizeof(*task));
    if (task == NULL)
        return NULL;

    task->info = kzalloc(sizeof(union kthread));
    if (task->info == NULL) {
        kfree(task);
        return NULL;
    }

    task->stack = (void *)task->info + sizeof(union kthread);
    task->eip = (void *)fn;

    return task;
}

void context_switch(volatile struct task *old, volatile struct task *new) {
    extern void context_switch_(volatile struct task * old,
                                volatile struct task * new);
    current = new;
    context_switch_(old, new);
}
