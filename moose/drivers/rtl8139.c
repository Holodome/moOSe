#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <assert.h>
#include <bitops.h>
#include <drivers/pci.h>
#include <drivers/rtl8139.h>
#include <errno.h>
#include <kstdio.h>
#include <net/inet.h>
#include <net/netdaemon.h>
#include <param.h>
#include <sched/locks.h>
#include <string.h>

#define RTL_REG_MAC0 0x00
#define TRL_REG_TX_STATUS 0x10
#define RTL_REG_TX_ADDR 0x20
#define RTL_REG_RX_BUFFER 0x30
#define RTL_REG_CMD 0x37
#define RTL_REG_CAPR 0x38
#define RTL_REG_INT_MASK 0x3c
#define RTL_REG_INT_STATUS 0x3e
#define RTL_REG_TX_CONFIG 0x40
#define RTL_REG_RX_CONFIG 0x44
#define RTL_REG_CONFIG1 0x52

#define RTL_ROK BIT(0)
#define RTL_RER BIT(1)
#define RTL_TOK BIT(2)
#define RTL_TER BIT(3)

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
    spinlock_t lock;
} rtl8139;

static void rtl8139_receive(void) {
    u8 frame[ETH_FRAME_MAX_SIZE];

    // check that rx buffer is not empty
    while ((port_in8(rtl8139.io_addr + RTL_REG_CMD) & 0x1) != 1) {
        u8 *rx_ptr = rtl8139.rx_buffer + rtl8139.rx_offset;
        u16 frame_size = *((u16 *)(rx_ptr + 2)) - 4;

        // skip 4 bytes rtl header
        rx_ptr += 4;
        if (rx_ptr + frame_size >= rtl8139.rx_buffer + RX_BUFFER_SIZE) {
            u16 part_size = (rtl8139.rx_buffer + RX_BUFFER_SIZE) - rx_ptr;
            memcpy(frame, rx_ptr, part_size);
            memcpy(frame + part_size, rtl8139.rx_buffer,
                   frame_size - part_size);
        } else {
            memcpy(frame, rx_ptr, frame_size);
        }

        net_daemon_add_frame(frame, frame_size);

        rtl8139.rx_offset = (rtl8139.rx_offset + frame_size + 4 + 4 + 3) & ~0x3;
        rtl8139.rx_offset %= RX_BUFFER_SIZE;

        port_out16(rtl8139.io_addr + RTL_REG_CAPR, rtl8139.rx_offset - 0x10);
    }
}

static void rtl8139_handler(struct isr_context *regs
                            __unused) {
    u16 isr = port_in16(rtl8139.io_addr + RTL_REG_INT_STATUS);

    if (isr & RTL_RER || isr & RTL_TER) {
        panic("rtl8139 tx/rx error\n");
    }

    if (isr & RTL_TOK) {
        kprintf("rtl8139 frame was sent\n");
    }

    if (isr & RTL_ROK) {
        rtl8139_receive();
        kprintf("rtl8139 frame was recieved\n");
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
    struct pci_device *dev =
        get_pci_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);
    if (dev == NULL) {
        kprintf("rtl8139 is not connected to pci bus\n");
        return -EIO;
    }

    int rc;
    if ((rc = enable_pci_device(dev))) {
        kprintf("failed to enable rtl8139\n");
        return rc;
    }

    init_spin_lock(&rtl8139.lock);

    // find io port resource
    struct resource *res = NULL;
    for (size_t i = 0; i < dev->resource_count; i++) {
        if (dev->resources[i]->kind == PORT_RESOURCE) {
            res = dev->resources[i];
            break;
        }
    }

    if (res == NULL) {
        release_pci_device(dev);
        return -EIO;
    }

    u32 io_addr = res->base;

    rtl8139.io_addr = io_addr;
    rtl8139.dev = dev;
    rtl8139.tx_index = 0;

    read_mac_addr(mac_addr);

    // pci bus mastering
    u32 command = read_pci_config_u16(dev->bdf, PCI_COMMAND);
    command |= BIT(2);
    write_pci_config_u16(dev->bdf, PCI_COMMAND, command);

    // power on device
    port_out8(io_addr + RTL_REG_CONFIG1, 0x0);

    // soft reset
    port_out8(io_addr + RTL_REG_CMD, 0x10);
    while ((port_in8(io_addr + RTL_REG_CMD) & 0x10) != 0) {
    }

    // set rx buffer
    port_out32(io_addr + RTL_REG_RX_BUFFER,
               ADDR_TO_PHYS((u64)rtl8139.rx_buffer));

    // enable rx and tx interrupts (bit 0 and 2)
    port_out16(io_addr + RTL_REG_INT_MASK, RTL_ROK | RTL_TOK);

    // rx buffer config (AB+AM+APM+AAP), nowrap
    port_out32(io_addr + RTL_REG_RX_CONFIG, 0xf);

    // disable loopback
    u32 tx_config = port_in32(io_addr + RTL_REG_TX_CONFIG);
    tx_config &= ~(BIT(17) | BIT(18));
    port_out32(io_addr + RTL_REG_TX_CONFIG, tx_config);

    port_out8(io_addr + RTL_REG_CMD, 0x0c);

    u8 isr = dev->interrupt_line;
    register_isr(isr, rtl8139_handler);

    return 0;
}

void destroy_rtl8139(void) {
    release_pci_device(rtl8139.dev);
}

void rtl8139_send(const void *frame, size_t size) {
    if (size > ETH_FRAME_MAX_SIZE) {
        kprintf("rtl8139 send error: invalid frame size\n");
        return;
    }

    u64 flags;
    spin_lock_irqsave(&rtl8139.lock, flags);

    memcpy(rtl8139.tx_buffer, frame, size);

    u8 tx_offset = sizeof(u32) * rtl8139.tx_index++;
    port_out32(rtl8139.io_addr + RTL_REG_TX_ADDR + tx_offset,
               ADDR_TO_PHYS((u64)rtl8139.tx_buffer));

    rtl8139.tx_index &= 0x3;

    u16 tx_status = rtl8139.io_addr + TRL_REG_TX_STATUS + tx_offset;
    port_out32(tx_status, size);

    spin_unlock_irqrestore(&rtl8139.lock, flags);
}
