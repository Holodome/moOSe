#include <drivers/disk.h>
#include <arch/cpu.h>
#include <idle.h>
#include <shell.h>

void idle_task(void) {
    init_disk();
    init_shell();

    for (;;)
        wait_for_int();
}
