#include <disk.h>
#include <idle.h>
#include <kstdio.h>
#include <kthread.h>
#include <shell.h>

__attribute__((noreturn)) void other_task(void) {
    for (;;)
        kprintf("other hello\n");
}

void idle_task(void) {
    init_disk();
    init_shell();

    launch_task(other_task);

    for (;;) {
        kprintf("hello world\n");
    }
}
