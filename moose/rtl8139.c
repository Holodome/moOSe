#include <arch/amd64/asm.h>
#include <mm/kmalloc.h>
#include <rtl8139.h>
#include <kstdio.h>
#include <param.h>
#include <list.h>
#include <pci.h>

#define RTL_REG_MAC0            0x0
#define RTL_REG_MAR0	        0x08
#define RTL_REG_RX_BUFFER       0x30
#define RTL_REG_CMD		0x37
#define RTL_REG_INT_MASK        0x3c
#define RTL_REG_RX_CONFIG       0x44
#define RTL_REG_CONFIG1         0x52

#define RX_BUFFER_SIZE (8192 + 16)
#define TX_BUFFER_SIZE 1518

static char rx_buffer[RX_BUFFER_SIZE];
//static char tx_buffer[TX_BUFFER_SIZE];

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

    // pci bus mastering
    u32 command = read_pci_config_u16(rtl8139->bdf, PCI_COMMAND);
    command |= (1 << 2);
    write_pci_config_u16(rtl8139->bdf, PCI_COMMAND, command);

    // power on device
    port_out8(ioaddr + RTL_REG_CONFIG1, 0x0);

    // soft reset
    port_out8(ioaddr + RTL_REG_CMD, 0x10);
    while((port_in8(ioaddr + RTL_REG_CMD) & 0x10) != 0)
        ;

    // set rx buffer
    port_out32(ioaddr + RTL_REG_RX_BUFFER, ADDR_TO_PHYS((u64)rx_buffer));

    // enable rx and tx interrupts (bit 0 and 2)
    port_out16(ioaddr + RTL_REG_INT_MASK, 0x0005);

    // rx buffer config (AB+AM+APM+AAP), nowrap
    port_out8(ioaddr + RTL_REG_RX_CONFIG, 0xf);

    kprintf("%x\n", rtl8139->command);

    kprintf("%x %x\n", rtl8139->interrupt_pin, rtl8139->interrupt_line);

    u8 interrupt = read_pci_config_u8(rtl8139->bdf, PCI_INTERRUPT_LINE);
    kprintf("%#x\n", interrupt);

    return 0;
}
