#include <arch/amd64/asm.h>
#include <pci.h>
#include <kstdio.h>
#include <mm/kmalloc.h>

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA    0xcfc

// bit 31
#define PCI_ENABLE_BIT 0x80000000
#define PCI_DEVICE_NOT_EXIST 0xffff

#define PCI_BRIDGE_CLASS 0x6
#define PCI_BRIDGE_SUB_CLASS 0x4

// PCI configuration space registers
#define PCI_VENDOR_ID		0x00	/* 16 bits */
#define PCI_DEVICE_ID		0x02	/* 16 bits */
#define PCI_COMMAND		0x04	/* 16 bits */
#define PCI_STATUS		0x06	/* 16 bits */
#define PCI_REVISION_ID		0x08	/* Revision ID */
#define PCI_PROG_IF		0x09	/* Reg. Level Programming Interface */
#define PCI_SUBCLASS     	0x0a	/* Device subclass */
#define PCI_CLASS               0x0b    /* Device class */
#define PCI_HEADER_TYPE		0x0e	/* 8 bits */

#define PCI_BASE_ADDRESS_0	0x10	/* 32 bits */
#define PCI_BASE_ADDRESS_1	0x14	/* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2	0x18	/* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3	0x1c	/* 32 bits */
#define PCI_BASE_ADDRESS_4	0x20	/* 32 bits */
#define PCI_BASE_ADDRESS_5	0x24	/* 32 bits */

#define PCI_SUB_VENDOR          0x2c
#define PCI_SUB_SYSTEM          0x2e
#define PCI_INTERRUPT_LINE      0x3c
#define PCI_INTERRUPT_PIN       0x3d

#define PCI_SECONDARY_BUS       0x19
#define PCI_SUBORDINATE_BUS     0x20

void io_wait(void) {
    port_out32(0x80, 0);
}

u8 read_pci_config_u8(u8 bus, u8 device, u8 function, u8 offset) {
    u32 addr = PCI_ENABLE_BIT | (bus << 16) | (device << 11) |
               (function << 8) | (offset & 0xf8);
    port_out32(PCI_CONFIG_ADDRESS, addr);

    io_wait();
    u8 data = (u8)((port_in32(PCI_CONFIG_DATA) >>
                      ((offset & 3) * 8)) & 0xff);

    return data;
}

u16 read_pci_config_u16(u8 bus, u8 device, u8 function, u8 offset) {
    u32 addr = PCI_ENABLE_BIT | (bus << 16) | (device << 11) |
               (function << 8) | (offset & 0xf8);
    port_out32(PCI_CONFIG_ADDRESS, addr);

    io_wait();
    u16 data = (u16)((port_in32(PCI_CONFIG_DATA) >>
                      ((offset & 2) * 8)) & 0xffff);

    return data;
}

u32 read_pci_config_u32(u8 bus, u8 device, u8 function, u8 offset) {
    u32 addr = PCI_ENABLE_BIT | (bus << 16) | (device << 11) |
               (function << 8) | (offset & 0xf8);
    port_out32(PCI_CONFIG_ADDRESS, addr);

    io_wait();
    u32 data = port_in32(PCI_CONFIG_DATA);

    return data;
}

void write_pci_config_u8(u8 bus, u8 device, u8 function, u8 offset, u8 data) {
    u32 addr = PCI_ENABLE_BIT | (bus << 16) | (device << 11) |
               (function << 8) | (offset & 0xf8);
    port_out32(PCI_CONFIG_ADDRESS, addr);
    io_wait();

    u32 temp = (u32)data << ((offset & 3) * 8);

    port_out32(PCI_CONFIG_DATA, temp);
    io_wait();
}

void write_pci_config_u16(u8 bus, u8 device, u8 function, u8 offset, u16 data) {
    u32 addr = PCI_ENABLE_BIT | (bus << 16) | (device << 11) |
               (function << 8) | (offset & 0xf8);
    port_out32(PCI_CONFIG_ADDRESS, addr);
    io_wait();

    u32 temp = (u32)data << ((offset & 2) * 8);

    port_out32(PCI_CONFIG_DATA, temp);
    io_wait();
}

void write_pci_config_u32(u8 bus, u8 device, u8 function, u8 offset, u32 data) {
    u32 addr = PCI_ENABLE_BIT | (bus << 16) | (device << 11) |
               (function << 8) | (offset & 0xf8);
    port_out32(PCI_CONFIG_ADDRESS, addr);
    io_wait();

    port_out32(PCI_CONFIG_DATA, data);
    io_wait();
}

int pci_is_bridge(struct pci_device *device) {
    return device->class_code == PCI_BRIDGE_CLASS &&
           device->subclass == PCI_BRIDGE_SUB_CLASS;
}

static void read_common_header(struct pci_device *device) {
    u8 bus_idx = device->bus->index;
    u8 device_idx = device->device_index;
    u8 func_idx = device->func_index;

    device->vendor = read_pci_config_u16(
        bus_idx, device_idx, func_idx, PCI_VENDOR_ID);
    device->device = read_pci_config_u16(
        bus_idx, device_idx, func_idx, PCI_DEVICE_ID);
    device->command = read_pci_config_u16(
        bus_idx, device_idx, func_idx, PCI_COMMAND);
    device->prog_if = read_pci_config_u8(
        bus_idx, device_idx, func_idx, PCI_PROG_IF);
    device->subclass = read_pci_config_u8(
        bus_idx, device_idx, func_idx, PCI_SUBCLASS);
    device->class_code = read_pci_config_u8(
        bus_idx, device_idx, func_idx, PCI_CLASS);
    device->hdr_type = read_pci_config_u8(
        bus_idx, device_idx, func_idx, PCI_HEADER_TYPE);

    device->interrupt_line = read_pci_config_u16(
        bus_idx, device_idx, func_idx, PCI_INTERRUPT_LINE);
    device->interrupt_pin = read_pci_config_u16(
        bus_idx, device_idx, func_idx, PCI_INTERRUPT_PIN);
}

static struct pci_device *create_device(struct pci_bus *bus,
                                        u8 device_idx, u8 func_idx) {
    struct pci_device *device = kzalloc(sizeof(struct pci_device));
    if (device == NULL)
        return NULL;

    u8 bus_idx = bus->index;
    device->device_index = device_idx;
    device->func_index = func_idx;

    device->bus = bus;
    read_common_header(device);

    if (pci_is_bridge(device)) {
        device->secondary_bus = read_pci_config_u8(
            bus_idx, device_idx, func_idx, PCI_SECONDARY_BUS);
        device->subordinate_bus = read_pci_config_u8(
            bus_idx, device_idx, func_idx, PCI_SUBORDINATE_BUS);
    } else {
        device->sub_vendor_id = read_pci_config_u16(
            bus_idx, device_idx, func_idx, PCI_SUB_VENDOR);
        device->subsystem_id = read_pci_config_u16(
            bus_idx, device_idx, func_idx, PCI_SUB_SYSTEM);
    }

    return device;
}

static struct pci_bus *scan_bus(u8 bus_idx) {
    struct pci_bus *bus = kzalloc(sizeof(struct pci_bus));
    if (bus == NULL)
        return NULL;

    init_list_head(&bus->list);

    for (u8 device_idx = 0; device_idx < 32; device_idx++) {
        for (u8 func_idx = 0; func_idx < 8; func_idx++) {
            u16 vendor_id = read_pci_config_u16(
                bus_idx, device_idx, func_idx, PCI_VENDOR_ID);
            if (vendor_id == PCI_DEVICE_NOT_EXIST)
                continue;

            struct pci_device *device =
                create_device(bus, device_idx, func_idx);
            if (device) {
                list_add(&device->list, &bus->devices);

                if (pci_is_bridge(device)) {
                    struct pci_bus *sub_bus = scan_bus(device->secondary_bus);
                    if (sub_bus) {
                        list_add(&sub_bus->list, &bus->children);
                    }
                }
            }
        }
    }

    return bus;
}

void init_pci(void) {

}
