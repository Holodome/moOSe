#include <drivers/rtl8139.h>
#include <drivers/pci.h>
#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <net/netdaemon.h>
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
#define RTL_REG_CAPR            0x38
#define RTL_REG_INT_MASK        0x3c
#define RTL_REG_INT_STATUS      0x3e
#define RTL_REG_TX_CONFIG       0x40
#define RTL_REG_RX_CONFIG       0x44
#define RTL_REG_CONFIG1         0x52
#define RTL_REG_MODE_CONTROL    0x62
#define RTL_REG_MODE_STATUS     0x64

#define RTL_ROK (1 << 0)
#define RTL_RER (1 << 1)
#define RTL_TOK (1 << 2)
#define RTL_TER (1 << 3)

#define RX_BUFFER_SIZE (8192 + 16)
// 1522 - 4 bytes for crc
#define TX_BUFFER_SIZE (ETH_FRAME_MAX_SIZE - 4)

static struct {
    u32 io_addr;
    u8 mac_addr[6];
    struct pci_device *dev;
    u8 rx_buffer[RX_BUFFER_SIZE] __attribute__((aligned(4)));
    u8 tx_buffer[TX_BUFFER_SIZE] __attribute__((aligned(4)));
    u8 tx_index;
    u16 rx_offset;
} rtl8139;

static void rtl8139_handler(struct registers_state *regs
                            __attribute__((unused))) {
    u16 isr = port_in16(rtl8139.io_addr + RTL_REG_INT_STATUS);

    if (isr & RTL_RER || isr & RTL_TER) {
        kprintf("int error\n");
    } else if (isr & RTL_ROK) {
        kprintf("int recieved\n");
        rtl8139_receive();
    } else if (isr & RTL_TOK) {
        kprintf("int sent\n");
    }

    port_out16(rtl8139.io_addr + RTL_REG_INT_STATUS, RTL_TOK | RTL_ROK);
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

void rtl8139_send(void *frame, u16 size) {
    memcpy(rtl8139.tx_buffer, frame, size);

    u8 tx_offset = sizeof(u32) * rtl8139.tx_index++;
    port_out32(rtl8139.io_addr + RTL_REG_TX_ADDR + tx_offset,
               ADDR_TO_PHYS((u64)rtl8139.tx_buffer));

    u16 tx_status = rtl8139.io_addr + TRL_REG_TX_STATUS + tx_offset;
    port_out32(tx_status, size);

    if(rtl8139.tx_index > 3)
        rtl8139.tx_index = 0;
}

void rtl8139_receive(void) {
    static u8 frame[ETH_FRAME_MAX_SIZE];
    // check that rx buffer is not empty
    while ((port_in8(rtl8139.io_addr + RTL_REG_CMD) & 0x1) != 1) {
        u8 *rx_ptr = rtl8139.rx_buffer + rtl8139.rx_offset;
        u16 frame_size = *((u16 *)(rx_ptr + 2));

        if (rx_ptr + frame_size >= rtl8139.rx_buffer + RX_BUFFER_SIZE) {
            u16 part_size = (rtl8139.rx_buffer + RX_BUFFER_SIZE) - rx_ptr;
            memcpy(frame, rx_ptr, part_size);
            memcpy(frame + part_size, rtl8139.rx_buffer, frame_size - part_size);
        } else {
            memcpy(frame, rx_ptr, frame_size);
        }

        debug_print_frame_hexdump(frame + 4, frame_size - 4);
        // frame without rtl buffer len (4 bytes)
        net_daemon_add_frame(frame + 4, frame_size - 4);

        kprintf("size = %d\n", frame_size);

        rtl8139.rx_offset = (rtl8139.rx_offset + frame_size + 4 + 3) & ~0x3;
        rtl8139.rx_offset %= RX_BUFFER_SIZE;

        port_out16(rtl8139.io_addr + RTL_REG_CAPR, rtl8139.rx_offset - 0x10);
    }
}
