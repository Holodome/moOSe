#include <kthread.h>
#include <kmalloc.h>
#include <arch/cpu.h>
#include <kernel.h>
#include <kstdio.h>

static LIST_HEAD(tasks);

int init_kinit_thread(void) {
    union kthread *thread = kzalloc(sizeof(union kthread));
    if (!thread) 
        return -1;

    set_stack((u64)(thread + 1), FIXUP_ADDR(0x90000));
    return 0;
}

