#include <drivers/rtl8139.h>
#include <drivers/pci.h>
#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <net/common.h>
#include <net/inet.h>
#include <endian.h>
#include <kstdio.h>
#include <param.h>

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
#define TX_BUFFER_SIZE (ETH_FRAME_MAX_SIZE - 4)

// 64 bytes - 4 bytes for crc
#define MIN_FRAME_SIZE (ETH_FRAME_MIN_SIZE - 4)

static struct {
    u32 io_addr;
    u8 mac_addr[6];
    struct pci_device *dev;
    u8 rx_buffer[RX_BUFFER_SIZE] __attribute__((aligned(32)));
    u8 tx_buffer[TX_BUFFER_SIZE] __attribute__((aligned(32)));
    u8 tx_index;
} rtl8139;

static void rtl8139_handler(struct registers_state *regs
                            __attribute__((unused))) {
    u16 isr = port_in16(rtl8139.io_addr + RTL_REG_INT_STATUS);

    if (isr & RTL_RER || isr & RTL_TER) {
        kprintf("int error\n");
    } else if (isr & RTL_ROK) {
        kprintf("int recieved\n");
    } else if (isr & RTL_TOK) {
        kprintf("int sent\n");
    }

    port_out16(rtl8139.io_addr + RTL_REG_INT_STATUS, 0x5);
}

static void read_mac_addr(u8 *mac_addr) {
    u32 mac1 = port_in32(rtl8139.io_addr + RTL_REG_MAC0);
    u32 mac2 = port_in32(rtl8139.io_addr + RTL_REG_MAC0 + 4);

    mac_addr[0] = mac1 >> 0;
    mac_addr[1] = mac1 >> 8;
    mac_addr[2] = mac1 >> 16;
    mac_addr[3] = mac1 >> 24;
    mac_addr[4] = mac2 >> 0;
    mac_addr[5] = mac2 >> 8;
}

int init_rtl8139(u8 *mac_addr) {
    if (mac_addr == NULL)
        return -1;

    struct pci_device *dev = get_pci_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);
    if (dev == NULL) {
        kprintf("rtl8139 is not connected to pci bus\n");
        return -1;
    }

    if (enable_pci_device(dev)) {
        kprintf("failed to enable rtl8139\n");
        return -1;
    }

    // io port resource
    struct resource *res = dev->resources[0];
    if (res == NULL)
        return -1;

    u32 io_addr = res->base;

    rtl8139.io_addr = io_addr;
    rtl8139.dev = dev;
    rtl8139.tx_index = 0;

    read_mac_addr(mac_addr);

    // pci bus mastering
    u32 command = read_pci_config_u16(dev->bdf, PCI_COMMAND);
    command |= (1 << 2);
    write_pci_config_u16(dev->bdf, PCI_COMMAND, command);

    // power on device
    port_out8(io_addr + RTL_REG_CONFIG1, 0x0);

    // soft reset
    port_out8(io_addr + RTL_REG_CMD, 0x10);
    while((port_in8(io_addr + RTL_REG_CMD) & 0x10) != 0)
        ;

    // set rx buffer
    port_out32(io_addr + RTL_REG_RX_BUFFER, ADDR_TO_PHYS((u64)rtl8139.rx_buffer));

    // enable rx and tx interrupts (bit 0 and 2)
    port_out16(io_addr + RTL_REG_INT_MASK, RTL_ROK | RTL_TOK);

    // rx buffer config (AB+AM+APM+AAP), nowrap
    port_out32(io_addr + RTL_REG_RX_CONFIG, 0xf);

    // disable loopback
    u32 tx_config = port_in32(io_addr + RTL_REG_TX_CONFIG);
    tx_config &= ~((1 << 17) | (1 << 18));
    port_out32(io_addr + RTL_REG_TX_CONFIG, tx_config);

    port_out8(io_addr + RTL_REG_CMD, 0x0c);

    u8 isr = dev->interrupt_line;
    register_isr(isr, rtl8139_handler);

    return 0;
}

void rtl8139_send(u8 *dst_mac, u16 eth_type, void *payload, u16 size) {
    struct eth_header *header = (struct eth_header*)rtl8139.tx_buffer;

    memcpy(header->dst_mac, dst_mac, sizeof(header->dst_mac));
    memcpy(header->src_mac, rtl8139.mac_addr, sizeof(header->src_mac));
    header->eth_type = htobe16(eth_type);

    memcpy(rtl8139.tx_buffer + sizeof(*header), payload, size);

    u16 frame_size = sizeof(*header) + size;
    // pad frame with zeros, to be at least mininum size
    if (frame_size < MIN_FRAME_SIZE) {
        memset(rtl8139.tx_buffer + frame_size, 0, MIN_FRAME_SIZE - frame_size);
        frame_size = MIN_FRAME_SIZE;
    }

    debug_print_frame_hexdump(rtl8139.tx_buffer, frame_size);

    u8 tx_offset = sizeof(u32) * rtl8139.tx_index++;
    port_out32(rtl8139.io_addr + RTL_REG_TX_ADDR + tx_offset,
               ADDR_TO_PHYS((u64)rtl8139.tx_buffer));

    u16 tx_status = rtl8139.io_addr + TRL_REG_TX_STATUS + tx_offset;
    port_out32(tx_status, frame_size);

    if(rtl8139.tx_index > 3)
        rtl8139.tx_index = 0;
}
