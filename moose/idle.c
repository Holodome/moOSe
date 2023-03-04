#include <disk.h>
#include <idle.h>
#include <arch/cpu.h>
#include <kstdio.h>
#include <kthread.h>
#include <shell.h>

void idle_task(void) {
    init_disk();
    init_shell();

    for (;;)
        wait_for_int();
}
