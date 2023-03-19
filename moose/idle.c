#include <arch/cpu.h>
#include <drivers/disk.h>
#include <idle.h>
#include <shell.h>

#include <fs/ext2.h>
#include <fs/vfs.h>

void idle_task(void) {
    init_disk();
    init_shell();

    /* struct filesystem ext2_fs = {.name = "ext2", .mount = ext2_mount}; */
    /* register_filesystem(&ext2_fs); */

    for (;;) wait_for_int();
}
