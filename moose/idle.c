#include <arch/cpu.h>
#include <blk_device.h>
#include <drivers/disk.h>
#include <idle.h>
#include <shell.h>

#include <fs/ext2.h>
#include <fs/vfs.h>

void idle_task(void) {
    init_disk();
    init_shell();

    print_blk_device(disk_part_dev);
    print_blk_device(disk_part1_dev);
    struct superblock *sb = vfs_mount(disk_part1_dev, ext2_mount);
    struct dentry *root_dentry = sb->root;
    struct file *root_file = vfs_open_dentry(root_dentry);
    for (;;) {
        struct dentry *read = vfs_readdir(root_file);
        if (PTR_ERR(read) == -ENOENT) break;

        kprintf("file %s\n", read->name);
    }

    kprintf("finished");

    for (;;) wait_for_int();
}
