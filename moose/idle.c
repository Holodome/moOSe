#include <arch/amd64/rtc.h>
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
#include <time.h>
#include <tty/console.h>
#include <tty/vga_console.h>
#include <tty/vterm.h>

void idle_task(void) {
    init_rtc();
    init_disk();

    for (;;) {
        wait_for_int();
    }
}
