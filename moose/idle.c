#include <arch/cpu.h>
#include <drivers/disk.h>
#include <idle.h>
#include <shell.h>
#include <blk_device.h>

#include <fs/ext2.h>
#include <fs/vfs.h>

void idle_task(void) {
    init_disk();
    init_shell();

    print_blk_device(disk_part_dev);
    print_blk_device(disk_part1_dev);
    /* struct superblock *sb = vfs_mount(disk_part1_dev, ext2_mount); */
    /* (void)sb; */

    for (;;) wait_for_int();
}
