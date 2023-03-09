#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <mm/kmalloc.h>
#include <rtl8139.h>
#include <kstdio.h>
#include <param.h>
#include <pci.h>

#define RTL_REG_MAC0            0x0
#define RTL_REG_MAR0	        0x08
#define RTL_REG_RX_BUFFER       0x30
#define RTL_REG_CMD		0x37
#define RTL_REG_INT_MASK        0x3c
#define RTL_REG_INT_STATUS      0x3e
#define RTL_REG_RX_CONFIG       0x44
#define RTL_REG_CONFIG1         0x52

#define RX_BUFFER_SIZE (8192 + 16)
#define TX_BUFFER_SIZE 1518

static struct {
    u32 ioaddr;
    u8 mac_addr[6];
    struct pci_device *dev;
    char rx_buffer[RX_BUFFER_SIZE];
    char tx_buffer[TX_BUFFER_SIZE];
} rtl8139;

static void rtl8139_handler(struct registers_state *regs
                            __attribute__((unused))) {
    write_pci_config_u16(
        rtl8139.dev->bdf, rtl8139.ioaddr + RTL_REG_INT_STATUS, 0x5);
    kprintf("interrupt is fired\n");
}

void debug_print_mac_addr(void) {
    u32 mac1 = port_in32(rtl8139.ioaddr + RTL_REG_MAC0);
    u32 mac2 = port_in32(rtl8139.ioaddr + RTL_REG_MAC0 + 4);
    rtl8139.mac_addr[0] = mac1 >> 0;
    rtl8139.mac_addr[1] = mac1 >> 8;
    rtl8139.mac_addr[2] = mac1 >> 16;
    rtl8139.mac_addr[3] = mac1 >> 24;

    rtl8139.mac_addr[4] = mac2 >> 0;
    rtl8139.mac_addr[5] = mac2 >> 8;
    kprintf("MAC: %01x:%01x:%01x:%01x:%01x:%01x\n",
            rtl8139.mac_addr[0], rtl8139.mac_addr[1],
            rtl8139.mac_addr[2], rtl8139.mac_addr[3],
            rtl8139.mac_addr[4], rtl8139.mac_addr[5]);
}

int init_rtl8139(void) {
    struct pci_device *dev = get_pci_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);
    if (dev == NULL) {
        kprintf("rtl8139 is not connected\n");
        return -1;
    }

    if (enable_pci_device(dev)) {
        kprintf("failed to enable rtl8139\n");
        return -1;
    }

    struct resource *res = dev->resources[0];
    if (res == NULL)
        return -1;

    u32 ioaddr = res->base;

    rtl8139.ioaddr = ioaddr;
    rtl8139.dev = dev;

    // pci bus mastering
    u32 command = read_pci_config_u16(dev->bdf, PCI_COMMAND);
    command |= (1 << 2);
    write_pci_config_u16(dev->bdf, PCI_COMMAND, command);

    // power on device
    port_out8(ioaddr + RTL_REG_CONFIG1, 0x0);

    // soft reset
    port_out8(ioaddr + RTL_REG_CMD, 0x10);
    while((port_in8(ioaddr + RTL_REG_CMD) & 0x10) != 0)
        ;

    // set rx buffer
    port_out32(ioaddr + RTL_REG_RX_BUFFER, ADDR_TO_PHYS((u64)rtl8139.rx_buffer));

    // enable rx and tx interrupts (bit 0 and 2)
    port_out16(ioaddr + RTL_REG_INT_MASK, 0x0005);

    // rx buffer config (AB+AM+APM+AAP), nowrap
    port_out8(ioaddr + RTL_REG_RX_CONFIG, 0xf);

    port_out8(ioaddr + RTL_REG_CMD, 0x0c);

    u8 isr = dev->interrupt_line;
    register_isr(isr, rtl8139_handler);

    return 0;
}
