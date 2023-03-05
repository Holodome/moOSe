#include <arch/amd64/asm.h>
#include <pci.h>
#include <kstdio.h>
#include <mm/kmalloc.h>
#include <mm/resource.h>

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA    0xcfc

// bit 31
#define PCI_ENABLE_BIT 0x80000000
#define PCI_DEVICE_NOT_EXIST 0xffff

#define PCI_BRIDGE_CLASS 0x6
#define PCI_BRIDGE_SUB_CLASS 0x4

// PCI configuration space registers
#define PCI_VENDOR_ID		0x00
#define PCI_DEVICE_ID		0x02
#define PCI_COMMAND		0x04
#define PCI_STATUS		0x06
#define PCI_PROG_IF		0x09
#define PCI_SUBCLASS     	0x0a
#define PCI_CLASS               0x0b
#define PCI_HEADER_TYPE		0x0e

#define PCI_BASE_ADDRESS_0	0x10
#define PCI_BASE_ADDRESS_1	0x14
#define PCI_BASE_ADDRESS_2	0x18
#define PCI_BASE_ADDRESS_3	0x1c
#define PCI_BASE_ADDRESS_4	0x20
#define PCI_BASE_ADDRESS_5	0x24

#define PCI_SUB_VENDOR          0x2c
#define PCI_SUB_SYSTEM          0x2e
#define PCI_INTERRUPT_LINE      0x3c
#define PCI_INTERRUPT_PIN       0x3d

#define PCI_SECONDARY_BUS       0x19
#define PCI_SUBORDINATE_BUS     0x20

#define PCI_BRIDGE_BARS_COUNT   2

static struct pci_bus *root_bus;

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

int is_pci_bridge(struct pci_device *device) {
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
    device->status = read_pci_config_u16(
        bus_idx, device_idx, func_idx, PCI_STATUS);
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

    if (is_pci_bridge(device)) {
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

    init_list_head(&bus->children);
    init_list_head(&bus->devices);
    bus->index = bus_idx;

    for (u8 device_idx = 0; device_idx < 32; device_idx++) {
        for (u8 func_idx = 0; func_idx < 8; func_idx++) {
            u16 vendor_id = read_pci_config_u16(
                bus_idx, device_idx, func_idx, PCI_VENDOR_ID);
            if (vendor_id == PCI_DEVICE_NOT_EXIST)
                continue;

            struct pci_device *device =
                create_device(bus, device_idx, func_idx);
            if (device) {
                list_add_tail(&device->list, &bus->devices);

                if (is_pci_bridge(device)) {
                    struct pci_bus *sub_bus = scan_bus(device->secondary_bus);
                    if (sub_bus) {
                        list_add_tail(&sub_bus->list, &bus->children);
                    }

                    sub_bus->bridge = device;
                    sub_bus->parent = bus;
                }
            }
        }
    }

    return bus;
}

struct pci_bus *get_root_bus(void) {
    return root_bus;
}

int init_pci(void) {
    root_bus = scan_bus(0);
    if (root_bus == NULL)
        return -1;

    return 0;
}

int enable_pci_device(struct pci_device *device) {
    u8 bars_count = PCI_BARS_COUNT;
    if (is_pci_bridge(device))
        bars_count = PCI_BRIDGE_BARS_COUNT;

    u8 bus_idx = device->bus->index;
    u8 device_idx = device->device_index;
    u8 func_idx = device->func_index;

    init_list_head(&device->resources);

    for (u8 bar_idx = 0; bar_idx < bars_count; bar_idx++) {
        u8 bar_offset = PCI_BASE_ADDRESS_0 + bar_idx * sizeof(u32);
        u32 bar = read_pci_config_u32(bus_idx, device_idx, func_idx, bar_offset);

        u32 addr;
        // io and memory spaces
        if (bar & 1) {
            addr = bar & 0xfffffff8;
        } else {
            addr = bar & 0xfffffff0;
        }

        write_pci_config_u32(bus_idx, device_idx, func_idx, bar_offset, UINT_MAX);
        io_wait();
        u32 size = ~read_pci_config_u32(bus_idx, device_idx,
                                        func_idx, bar_offset) + 1;

        write_pci_config_u32(bus_idx, device_idx, func_idx, bar_offset, bar);
        io_wait();

        struct resource *res = request_port_region(addr, size);
        if (res) {
            list_add_tail(&res->list, &device->resources);
        }
    }

    return 0;
}

static struct pci_device *find_pci_device(struct pci_bus *bus, u16 vendor_id,
                                          u16 device_id) {
    struct pci_device *device = NULL;
    list_for_each_entry(device, &bus->devices, list) {
        if (device->vendor == vendor_id && device->device == device_id) {
            return device;
        }
    }

    struct pci_bus *sub_bus;
    list_for_each_entry(sub_bus, &bus->children, list) {
        device = find_pci_device(sub_bus, vendor_id, device_id);
        if (device)
            return device;
    }

    return device;
}

struct pci_device *get_pci_device(u16 vendor, u16 device) {
    if (root_bus == NULL)
        return NULL;

    return find_pci_device(root_bus, vendor, device);
}

static void debug_print_device(struct pci_device *device) {
    kprintf("DEV: bus=%d, dev=%d, func=%d, vendor=%x, devid=%x, "
            "class=%#x, subclass=%#x\n",
            device->bus->index, device->device_index, device->func_index,
            device->vendor, device->device, device->class_code, device->subclass);
}

void debug_print_bus(struct pci_bus *bus) {
    if (bus == NULL)
        return;

    kprintf("BUS: bus=%d\n", bus->index);

    struct pci_device *device;
    list_for_each_entry(device, &bus->devices, list) {
        debug_print_device(device);
    }

    struct pci_bus *sub_bus = NULL;
    list_for_each_entry(sub_bus, &bus->children, list) {
        debug_print_bus(sub_bus);
    }
}
