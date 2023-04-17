#include <arch/amd64/idt.h>
#include <assert.h>
#include <bitops.h>
#include <drivers/io_resource.h>
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

static struct rtl8139 {
    struct pci_device *pci;
    u32 io_addr;
    u8 mac_addr[6];
    struct io_resource *address_space;
    u8 rx_buffer[RX_BUFFER_SIZE] __attribute__((aligned(4)));
    u8 tx_buffer[TX_BUFFER_SIZE] __attribute__((aligned(4)));
    u8 tx_index;
    u16 rx_offset;
    spinlock_t lock;
} __rtl8139;

static void rtl8139_receive(struct rtl8139 *rtl8139) {
    u8 frame[ETH_FRAME_MAX_SIZE];

    // check that rx buffer is not empty
    while ((port_in8(rtl8139->io_addr + RTL_REG_CMD) & 0x1) != 1) {
        u8 *rx_ptr = rtl8139->rx_buffer + rtl8139->rx_offset;
        u16 frame_size;
        // note that frame_size is actually correctly aligned and we could as
        // well deference the pointer directly
        memcpy(&frame_size, rx_ptr + 2, sizeof(frame_size));
        frame_size -= 4;

        // skip 4 bytes rtl header
        rx_ptr += 4;
        if (rx_ptr + frame_size >= rtl8139->rx_buffer + RX_BUFFER_SIZE) {
            u16 part_size = (rtl8139->rx_buffer + RX_BUFFER_SIZE) - rx_ptr;
            memcpy(frame, rx_ptr, part_size);
            memcpy(frame + part_size, rtl8139->rx_buffer,
                   frame_size - part_size);
        } else {
            memcpy(frame, rx_ptr, frame_size);
        }

        net_daemon_add_frame(frame, frame_size);

        rtl8139->rx_offset =
            (rtl8139->rx_offset + frame_size + 4 + 4 + 3) & ~0x3;
        rtl8139->rx_offset %= RX_BUFFER_SIZE;

        port_out16(rtl8139->io_addr + RTL_REG_CAPR, rtl8139->rx_offset - 0x10);
    }
}

static void rtl8139_handler(struct registers_state *) {
    u16 status = port_in16(__rtl8139.io_addr + RTL_REG_INT_STATUS);

    if (status & RTL_RER || status & RTL_TER) {
        panic("rtl8139 tx/rx error\n");
    }

    if (status & RTL_TOK) {
        kprintf("rtl8139 frame was sent\n");
    }

    if (status & RTL_ROK) {
        rtl8139_receive(&__rtl8139);
    }

    port_out16(__rtl8139.io_addr + RTL_REG_INT_STATUS, RTL_TOK | RTL_ROK);
}

static void read_mac_addr(struct rtl8139 *rtl8139) {
    u32 mac1 = port_in32(rtl8139->io_addr + RTL_REG_MAC0);
    u32 mac2 = port_in32(rtl8139->io_addr + RTL_REG_MAC0 + 4);

    u8 *mac_addr = rtl8139->mac_addr;
    mac_addr[0] = mac1 >> 0;
    mac_addr[1] = mac1 >> 8;
    mac_addr[2] = mac1 >> 16;
    mac_addr[3] = mac1 >> 24;
    mac_addr[4] = mac2 >> 0;
    mac_addr[5] = mac2 >> 8;
}

void rtl8139_get_mac(u8 mac[static 6]) {
    memcpy(mac, __rtl8139.mac_addr, 6);
}

static void rtl8139_configure(struct rtl8139 *rtl8139) {
    struct pci_device *pci = rtl8139->pci;
    u32 io_addr = rtl8139->io_addr;
    read_mac_addr(rtl8139);

    // pci bus mastering
    u32 command = read_pci_config_u16(pci->bdf, PCI_COMMAND);
    command |= BIT(2);
    write_pci_config_u16(pci->bdf, PCI_COMMAND, command);

    // power on device
    port_out8(io_addr + RTL_REG_CONFIG1, 0x0);

    // soft reset
    port_out8(io_addr + RTL_REG_CMD, 0x10);
    while ((port_in8(io_addr + RTL_REG_CMD) & 0x10) != 0) {
    }

    // set rx buffer
    port_out32(io_addr + RTL_REG_RX_BUFFER,
               ADDR_TO_PHYS((u64)rtl8139->rx_buffer));

    // enable rx and tx interrupts (bit 0 and 2)
    port_out16(io_addr + RTL_REG_INT_MASK, RTL_ROK | RTL_TOK);

    // rx buffer config (AB+AM+APM+AAP), nowrap
    port_out32(io_addr + RTL_REG_RX_CONFIG, 0xf);

    // disable loopback
    u32 tx_config = port_in32(io_addr + RTL_REG_TX_CONFIG);
    tx_config &= ~(BIT(17) | BIT(18));
    port_out32(io_addr + RTL_REG_TX_CONFIG, tx_config);

    port_out8(io_addr + RTL_REG_CMD, 0x0c);

    u8 isr = pci->interrupt_line;
    register_isr(isr, rtl8139_handler);
}

int init_rtl8139(void) {
    struct pci_device *dev =
        get_pci_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);
    if (dev == NULL) {
        kprintf("rtl8139 is not connected to pci bus\n");
        return -EIO;
    }

    if (enable_pci_device(dev)) {
        kprintf("failed to enable rtl8139\n");
        release_pci_device(dev);
        return -EBUSY;
    }

    struct io_resource *res = NULL;
    for (size_t i = 0; i < dev->resource_count; i++) {
        if (dev->resources[i]->kind == IO_RES_PORT) {
            res = dev->resources[i];
            break;
        }
    }

    if (res == NULL) {
        kprintf("failed to find rtl8139 io port address space\n");
        release_pci_device(dev);
        return -EBUSY;
    }

    init_spin_lock(&__rtl8139.lock);
    __rtl8139.io_addr = res->base;
    __rtl8139.pci = dev;
    __rtl8139.tx_index = 0;
    rtl8139_configure(&__rtl8139);

    return 0;
}

void destroy_rtl8139(void) {
    release_pci_device(__rtl8139.pci);
}

void __rtl8139_send(struct rtl8139 *rtl8139, const void *frame, size_t size) {
    if (size > ETH_FRAME_MAX_SIZE) {
        kprintf("rtl8139 send error: invalid frame size\n");
        return;
    }

    cpuflags_t flags = spin_lock_irqsave(&rtl8139->lock);
    memcpy(rtl8139->tx_buffer, frame, size);

    size_t tx_offset = sizeof(u32) * rtl8139->tx_index++;
    port_out32(rtl8139->io_addr + RTL_REG_TX_ADDR + tx_offset,
               ADDR_TO_PHYS((u64)rtl8139->tx_buffer));
    rtl8139->tx_index &= 0x3;
    port_out32(rtl8139->io_addr + TRL_REG_TX_STATUS + tx_offset, size);

    spin_unlock_irqrestore(&rtl8139->lock, flags);
}

void rtl8139_send(const void *frame, size_t size) {
    return __rtl8139_send(&__rtl8139, frame, size);
}
