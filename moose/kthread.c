#include <kthread.h>
#include <kmalloc.h>
#include <arch/processor.h>
#include <kernel.h>
#include <kstdio.h>

int init_kinit_thread(void) {
    union kthread *thread = kzalloc(sizeof(union kthread));
    if (!thread) 
        return -1;

    set_stack((u64)(thread + 1), FIXUP_ADDR(0x90000));
    return 0;
}
