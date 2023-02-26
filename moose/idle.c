#include <idle.h>
#include <kstdio.h>
#include <disk.h>
#include <arch/amd64/rtc.h>
#include <shell.h>
#include <kthread.h>

__attribute__((noreturn)) void other_task(void) {
    for (;;)
        kprintf("other hello\n");
}

void idle_task(void) {
    init_disk();
    init_rtc();
    init_shell();

    launch_task(other_task);

    for (;;) {
        kprintf("hello world\n");
    }
}
