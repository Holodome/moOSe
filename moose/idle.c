#include <arch/cpu.h>
#include <blk_device.h>
#include <drivers/disk.h>
#include <idle.h>
#include <shell.h>
#include <string.h>

#include <fs/ext2.h>
#include <fs/vfs.h>

void idle_task(void) {
    init_disk();
    init_shell();

    print_blk_device(disk_part_dev);
    print_blk_device(disk_part1_dev);
    struct superblock *sb = vfs_mount(disk_part1_dev, ext2_mount);
    struct dentry *root_dentry = sb->root;
    print_inode(root_dentry->inode);
    struct file *root_file = vfs_open_dentry(root_dentry);
    for (;;) {
        struct dentry *read = vfs_readdir(root_file);
        if (PTR_ERR(read) == -ENOENT) break;

        struct inode *inode = read->inode;
        /* print_inode(inode); */
        if (S_ISREG(inode->mode)) {
            kprintf("file %s\n", read->name);
        } else if (S_ISDIR(inode->mode) && strcmp(read->name, ".") &&
                   strcmp(read->name, "..")) {

            struct file *dir_file = vfs_open_dentry(read);
            kprintf("dir %s\n", read->name);
            for (;;) {
                struct dentry *read1 = vfs_readdir(dir_file);
                if (PTR_ERR(read1) == -ENOENT) break;
                kprintf("in dir %s\n", read1->name);
            }
        }
    }

    kprintf("finished");

    for (;;) wait_for_int();
}
