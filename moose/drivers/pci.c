#include <arch/amd64/asm.h>
#include <arch/amd64/cpu.h>
#include <drivers/pci.h>
#include <kstdio.h>
#include <mm/kmalloc.h>
#include <mm/resource.h>
#include <assert.h>

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA    0xcfc

// bit 31
#define PCI_ENABLE_BIT 0x80000000
#define PCI_DEVICE_NOT_EXIST 0xffff

#define PCI_BRIDGE_CLASS 0x6
#define PCI_BRIDGE_SUB_CLASS 0x4

#define PCI_BRIDGE_BARS_COUNT 2

static struct pci_bus *root_bus;

u8 read_pci_config_u8(u32 bdf, u8 offset) {
    return read_pci_config_u32(bdf, offset) >> ((offset & 3) * 8);
}

u16 read_pci_config_u16(u32 bdf, u8 offset) {
    return read_pci_config_u32(bdf, offset) >> ((offset & 2) * 8);
}

u32 read_pci_config_u32(u32 bdf, u8 offset) {
    u32 addr = PCI_ENABLE_BIT | bdf | (offset & 0xfc);
    port_out32(PCI_CONFIG_ADDRESS, addr);

    io_wait();
    return port_in32(PCI_CONFIG_DATA);
}

void write_pci_config_u8(u32 bdf, u8 offset, u8 data) {
    u32 temp = read_pci_config_u32(bdf, offset);

    u32 shift = (offset & 3) * 8;
    temp &= ~(0xff << shift);
    temp |= data << shift;

    write_pci_config_u32(bdf, offset, temp);
}

void write_pci_config_u16(u32 bdf, u8 offset, u16 data) {
    u32 temp = read_pci_config_u32(bdf, offset);

    u32 shift = (offset & 2) * 8;
    temp &= ~(0xffff << shift);
    temp |= data << shift;

    write_pci_config_u32(bdf, offset, temp);
}

void write_pci_config_u32(u32 bdf, u8 offset, u32 data) {
    u32 addr = PCI_ENABLE_BIT | bdf | (offset & 0xfc);
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
    u32 bdf = BDF(bus_idx, device_idx, func_idx);

    device->vendor = read_pci_config_u16(bdf, PCI_VENDOR_ID);
    device->device = read_pci_config_u16(bdf, PCI_DEVICE_ID);
    device->command = read_pci_config_u16(bdf, PCI_COMMAND);
    device->status = read_pci_config_u16(bdf, PCI_STATUS);
    device->prog_if = read_pci_config_u8(bdf, PCI_PROG_IF);
    device->subclass = read_pci_config_u8(bdf, PCI_SUBCLASS);
    device->class_code = read_pci_config_u8(bdf, PCI_CLASS);
    device->hdr_type = read_pci_config_u8(bdf, PCI_HEADER_TYPE);

    device->interrupt_line = read_pci_config_u16(bdf, PCI_INTERRUPT_LINE);
    device->interrupt_pin = read_pci_config_u16(bdf, PCI_INTERRUPT_PIN);
}

static void init_device(struct pci_device *device, struct pci_bus *bus,
                                        u8 device_idx, u8 func_idx) {
    u8 bus_idx = bus->index;
    device->device_index = device_idx;
    device->func_index = func_idx;

    device->bus = bus;
    read_common_header(device);

    u32 bdf = BDF(bus_idx, device_idx, func_idx);
    device->bdf = bdf;
    if (is_pci_bridge(device)) {
        device->secondary_bus = read_pci_config_u8(bdf, PCI_SECONDARY_BUS);
        device->subordinate_bus = read_pci_config_u8(bdf, PCI_SUBORDINATE_BUS);
    } else {
        device->sub_vendor_id = read_pci_config_u16(bdf, PCI_SUB_VENDOR);
        device->subsystem_id = read_pci_config_u16(bdf, PCI_SUB_SYSTEM);
    }
}

static struct pci_bus *scan_bus(u8 bus_idx) {
    struct pci_bus *bus = kzalloc(sizeof(*bus));
    if (bus == NULL)
        return NULL;

    init_list_head(&bus->children);
    init_list_head(&bus->devices);
    bus->index = bus_idx;

    for (u8 device_idx = 0; device_idx < 32; device_idx++) {
        for (u8 func_idx = 0; func_idx < 8; func_idx++) {
            u32 bdf = BDF(bus_idx, device_idx, func_idx);
            u16 vendor_id = read_pci_config_u16(bdf, PCI_VENDOR_ID);
            if (vendor_id == PCI_DEVICE_NOT_EXIST)
                continue;

            struct pci_device *device = kzalloc(sizeof(*device));
            if (device == NULL)
                return NULL;

            init_device(device, bus, device_idx, func_idx);
            list_add_tail(&device->list, &bus->devices);

            if (is_pci_bridge(device)) {
                struct pci_bus *sub_bus = scan_bus(device->secondary_bus);
                if (sub_bus == NULL)
                    return NULL;

                sub_bus->bridge = device;
                sub_bus->parent = bus;

                list_add_tail(&sub_bus->list, &bus->children);
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
    u32 bdf = BDF(bus_idx, device_idx, func_idx);

    for (u8 bar_idx = 0, res_idx = 0; bar_idx < bars_count; bar_idx++) {
        u8 bar_offset = PCI_BASE_ADDRESS_0 + bar_idx * sizeof(u32);
        u64 bar = read_pci_config_u32(bdf, bar_offset);

        write_pci_config_u32(bdf, bar_offset, UINT_MAX);
        u32 size = ~read_pci_config_u32(bdf, bar_offset) + 1;
        write_pci_config_u32(bdf, bar_offset, bar);

        if (size == 0)
            continue;

        struct resource *res;
        if (bar & 1) {
            // io space bar
            u32 addr = bar & 0xfffffffc;
            res = request_port_region(addr, size);
        } else {
            // memory space bar
            u8 mem_type = (bar >> 1) & 3;

            u64 addr = bar & 0xfffffff0;
            // 64-bit memory bar
            if (mem_type == 0x2) {
                bar = read_pci_config_u32(bdf, bar_offset + 4);
                addr += (bar << 32);
                bar_idx++;
            }

            res = request_mem_region(addr, size);
        }

        if (res == NULL)
            return -1;

        device->resources[res_idx++] = res;
    }

    return 0;
}

static struct pci_device *find_pci_device(struct pci_bus *bus, u16 vendor_id,
                                          u16 device_id) {
    expects(bus != NULL);

    struct pci_device *device;
    list_for_each_entry(device, &bus->devices, list) {
        if (device->vendor == vendor_id && device->device == device_id) {
            return device;
        }
    }

    struct pci_bus *sub_bus;
    list_for_each_entry(sub_bus, &bus->children, list) {
        device = find_pci_device(sub_bus, vendor_id, device_id);
        if (device) {
            return device;
        }
    }

    return NULL;
}

struct pci_device *get_pci_device(u16 vendor, u16 device) {
    expects(root_bus != NULL);
    return find_pci_device(root_bus, vendor, device);
}

static void debug_print_device(struct pci_device *device) {
    kprintf("DEV: bus=%d, dev=%d, func=%d, vendor=%x, devid=%x, "
            "class=%#x, subclass=%#x\n",
            device->bus->index, device->device_index, device->func_index,
            device->vendor, device->device, device->class_code, device->subclass);
}

void debug_print_bus(struct pci_bus *bus) {
    expects(bus != NULL);
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
