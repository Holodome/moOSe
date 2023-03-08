#include <arch/amd64/asm.h>
#include <mm/physmem.h>
#include <mm/kmalloc.h>
#include <rtl8139.h>
#include <kstdio.h>
#include <list.h>
#include <pci.h>

#define RTL_REG_MAC0            0x0
#define RTL_REG_MAR0	        0x08
#define RTL_REG_RX_BUFFER       0x30
#define RTL_REG_CMD		0x37
#define RTL_REG_CONFIG1         0x52

#define RX_BUFFER_SIZE (8192 + 16)

static void *rx_buffer;

int init_rtl8139(void) {
    struct pci_device *rtl8139 = get_pci_device(
        RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);
    if (rtl8139 == NULL) {
        kprintf("rtl8139 is not connected\n");
        return -1;
    }

    if (enable_pci_device(rtl8139)) {
        kprintf("failed to enable rtl8139\n");
        return -1;
    }

    struct resource *res = list_next_or_null(
        &rtl8139->resources, &rtl8139->resources, struct resource, list);
    if (res == NULL)
        return -1;

    u32 ioaddr = res->base;

    port_out8(ioaddr + RTL_REG_CONFIG1, 0x0);
    port_out8(ioaddr + RTL_REG_CMD, 0x10);
    while((port_in8(ioaddr + RTL_REG_CMD) & 0x10) != 0)
        ;

    rx_buffer = kmalloc(RX_BUFFER_SIZE);
    if (rx_buffer == NULL)
        return -1;

    return 0;
}
