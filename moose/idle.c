#include <disk.h>
#include <idle.h>
#include <arch/cpu.h>
#include <shell.h>
#include <arch/amd64/keyboard.h>

void idle_task(void) {
    init_disk();
    init_shell();

    for (;;)
        wait_for_int();
}
