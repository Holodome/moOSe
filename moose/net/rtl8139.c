#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <mm/kmalloc.h>
#include <net/rtl8139.h>
#include <kstdio.h>
#include <param.h>
#include <pci.h>

#define RTL_REG_MAC0            0x0
#define RTL_REG_MAR0	        0x08
#define TRL_REG_TX_STATUS       0x10
#define RTL_REG_TX_ADDR         0x20
#define RTL_REG_RX_BUFFER       0x30
#define RTL_REG_CMD		0x37
#define RTL_REG_INT_MASK        0x3c
#define RTL_REG_INT_STATUS      0x3e
#define RTL_REG_TX_CONFIG       0x40
#define RTL_REG_RX_CONFIG       0x44
#define RTL_REG_CONFIG1         0x52

#define RTL_ROK (1 << 0)
#define RTL_RER (1 << 1)
#define RTL_TOK (1 << 2)
#define RTL_TER (1 << 3)

#define RX_BUFFER_SIZE (8192 + 16)
// 1522 - 4 bytes for crc
#define TX_BUFFER_SIZE 1518

// minimum 64 bytes - 4 bytes for crc
// because crc append function enabled
#define MIN_FRAME_SIZE 60

static struct {
    u32 ioaddr;
    u8 mac_addr[6];
    struct pci_device *dev;
    char rx_buffer[RX_BUFFER_SIZE];
    char tx_buffer[TX_BUFFER_SIZE];
    u8 tx_index;
} rtl8139;

static void rtl8139_handler(struct registers_state *regs
                            __attribute__((unused))) {
    u16 isr = port_in16(RTL_REG_INT_STATUS);
    kprintf("%x\n", isr);

    if (isr & RTL_RER || isr & RTL_TER) {
        kprintf("int error\n");
        port_out16(rtl8139.ioaddr + RTL_REG_INT_STATUS, 0x5);
        return;
    }

    if (isr & RTL_ROK) {
        kprintf("int recieved\n");
        port_out16(rtl8139.ioaddr + RTL_REG_INT_STATUS, 0x5);
        return;
    }

    if (isr & RTL_TOK) {
        kprintf("int sent\n");
        port_out16(rtl8139.ioaddr + RTL_REG_INT_STATUS, 0x5);
        return;
    }
}

void debug_print_mac_addr(void) {
    kprintf("MAC: %01x:%01x:%01x:%01x:%01x:%01x\n",
            rtl8139.mac_addr[0], rtl8139.mac_addr[1],
            rtl8139.mac_addr[2], rtl8139.mac_addr[3],
            rtl8139.mac_addr[4], rtl8139.mac_addr[5]);
}

static void read_mac_addr(void) {
    u32 mac1 = port_in32(rtl8139.ioaddr + RTL_REG_MAC0);
    u32 mac2 = port_in32(rtl8139.ioaddr + RTL_REG_MAC0 + 4);

    rtl8139.mac_addr[0] = mac1 >> 0;
    rtl8139.mac_addr[1] = mac1 >> 8;
    rtl8139.mac_addr[2] = mac1 >> 16;
    rtl8139.mac_addr[3] = mac1 >> 24;
    rtl8139.mac_addr[4] = mac2 >> 0;
    rtl8139.mac_addr[5] = mac2 >> 8;
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
    rtl8139.tx_index = 0;

    read_mac_addr();

    // pci bus mastering
    u32 command = read_pci_config_u16(dev->bdf, PCI_COMMAND);
    command |= (1 << 2);
    write_pci_config_u16(dev->bdf, PCI_COMMAND, command);

    u16 isr1 = port_in16(RTL_REG_INT_STATUS);
    kprintf("%x\n", isr1);

    // power on device
    port_out8(ioaddr + RTL_REG_CONFIG1, 0x0);

    // soft reset
    port_out8(ioaddr + RTL_REG_CMD, 0x10);
    while((port_in8(ioaddr + RTL_REG_CMD) & 0x10) != 0)
        ;

    // set rx buffer
    port_out32(ioaddr + RTL_REG_RX_BUFFER, ADDR_TO_PHYS((u64)rtl8139.rx_buffer));

    // enable rx and tx interrupts (bit 0 and 2)
    port_out16(ioaddr + RTL_REG_INT_MASK, RTL_ROK | RTL_TOK);

    // rx buffer config (AB+AM+APM+AAP), nowrap
    port_out8(ioaddr + RTL_REG_RX_CONFIG, 0xf);

    // disable loopback
    u32 tx_config = port_in32(RTL_REG_TX_CONFIG);
    tx_config &= ~((1 << 17) | (1 << 18));
    port_out32(RTL_REG_TX_CONFIG, tx_config);

    port_out8(ioaddr + RTL_REG_CMD, 0x0c);

    u8 isr = dev->interrupt_line;
    register_isr(isr, rtl8139_handler);

    return 0;
}

void rtl8139_send(u8 *dst_mac, void *data, u16 size) {
    struct eth_header *header = (struct eth_header*)rtl8139.tx_buffer;

    memcpy(header->dst_mac, dst_mac, sizeof(header->dst_mac));
    memcpy(header->src_mac, rtl8139.mac_addr, sizeof(header->src_mac));
    // FIXME: for example ip protocol
    header->eth_type = 0x0800;

    memcpy(rtl8139.tx_buffer + sizeof(*header), data, size);

    u16 frame_size = sizeof(*header) + size;
    // pad frame with zeros, to be at least mininum size
    if (frame_size < MIN_FRAME_SIZE) {
        memset(rtl8139.tx_buffer + frame_size, 0, MIN_FRAME_SIZE - frame_size);
        frame_size = MIN_FRAME_SIZE;
    }

    u8 tx_offset = sizeof(u32) * rtl8139.tx_index++;
    port_out32(rtl8139.ioaddr + RTL_REG_TX_ADDR + tx_offset,
               ADDR_TO_PHYS((u64)rtl8139.tx_buffer));

    u16 tx_status = rtl8139.ioaddr + TRL_REG_TX_STATUS + tx_offset;
    port_out32(tx_status, size);

    if(rtl8139.tx_index > 3)
        rtl8139.tx_index = 0;

    while (((port_in32(tx_status) >> 15) & 1) != 1)
        ;
}
