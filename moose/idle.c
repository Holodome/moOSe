#include <arch/cpu.h>
#include <blk_device.h>
#include <drivers/disk.h>
#include <drivers/pci.h>
#include <errno.h>
#include <fs/ext2.h>
#include <fs/vfs.h>
#include <idle.h>
#include <kstdio.h>
#include <net/arp.h>
#include <net/frame.h>
#include <net/icmp.h>
#include <net/inet.h>
#include <net/udp.h>
#include <panic.h>
#include <string.h>

void idle_task(void) {
    init_disk();

    if (init_pci())
        panic("failed to initialize pci bus\n");

    struct pci_bus *bus = get_root_bus();
    debug_print_bus(bus);

    if (init_inet())
        panic("failed to initialize inet system\n");

    u8 mac_addr[6];
    for (int i = 0; i < 5; i++) {
        if (arp_get_mac(gateway_ip_addr, mac_addr)) {
            kprintf("can't find mac for this ip address\n");
            halt_cpu();
        }
        kprintf("gateway ");
        debug_print_mac_addr(mac_addr);
        debug_clear_arp_cache();
    }

    if (arp_get_mac(dns_ip_addr, mac_addr))
        panic("can't find mac for this ip address\n");

    kprintf("dns ");
    debug_print_mac_addr(mac_addr);

    struct net_frame *frame = get_empty_send_net_frame();
    icmp_send_echo_request(frame, gateway_ip_addr);
    release_net_frame(frame);

    frame = get_empty_send_net_frame();
    char *message = "Hello world!";
    memcpy(frame->payload, message, strlen(message));
    frame->payload_size = strlen(message);
    frame->size = frame->payload_size;

    udp_send_frame(frame, gateway_ip_addr, 80, 80);
    release_net_frame(frame);

    print_blk_device(disk_part_dev);
    print_blk_device(disk_part1_dev);
    struct superblock *sb = vfs_mount(disk_part1_dev, ext2_mount);
    struct dentry *root_dentry = sb->root;
    print_inode(root_dentry->inode);
    struct file *root_file = vfs_open_dentry(root_dentry);
    for (;;) {
        struct dentry *read = vfs_readdir(root_file);
        if (PTR_ERR(read) == -ENOENT)
            break;

        struct inode *inode = read->inode;
        if (S_ISREG(inode->mode)) {
            kprintf("file %s\n", read->name);
        } else if (S_ISDIR(inode->mode) && strcmp(read->name, ".") &&
                   strcmp(read->name, "..")) {

            struct file *dir_file = vfs_open_dentry(read);
            kprintf("dir %s\n", read->name);
            for (;;) {
                struct dentry *read1 = vfs_readdir(dir_file);
                if (IS_PTR_ERR(read1) && PTR_ERR(read1) == -ENOENT)
                    break;
                kprintf("  in dir %s\n", read1->name);
            }
        }
    }

    for (;;)
        wait_for_int();
}
