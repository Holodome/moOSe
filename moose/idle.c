#include <disk.h>
#include <idle.h>
#include <kstdio.h>
#include <kthread.h>
#include <shell.h>

__attribute__((noreturn)) void other_task(void) {
    for (;;)
        ;
}

void idle_task(void) {
    init_disk();
    init_shell();

    launch_task(other_task);

    for (;;)
        ;
}
