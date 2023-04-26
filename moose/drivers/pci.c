#include <moose/arch/cpu.h>
#include <moose/bitops.h>
#include <moose/drivers/io_resource.h>
#include <moose/drivers/pci.h>
#include <moose/errno.h>
#include <moose/kstdio.h>
#include <moose/mm/kmalloc.h>

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA 0xcfc

#define PCI_ENABLE_BIT BIT(31)
#define PCI_DEVICE_NOT_EXIST 0xffff

#define PCI_BRIDGE_CLASS 0x6
#define PCI_BRIDGE_SUB_CLASS 0x4

#define PCI_BRIDGE_BARS_COUNT 2

static struct pci_bus *root_bus;

u8 read_pci_config_u8(u32 bdf, unsigned offset) {
    return read_pci_config_u32(bdf, offset) >> ((offset & 3) * 8);
}

u16 read_pci_config_u16(u32 bdf, unsigned offset) {
    return read_pci_config_u32(bdf, offset) >> ((offset & 2) * 8);
}

u32 read_pci_config_u32(u32 bdf, unsigned offset) {
    u32 addr = PCI_ENABLE_BIT | bdf | (offset & 0xfc);
    port_out32(PCI_CONFIG_ADDRESS, addr);

    io_wait();
    return port_in32(PCI_CONFIG_DATA);
}

void write_pci_config_u8(u32 bdf, unsigned offset, u8 data) {
    u32 temp = read_pci_config_u32(bdf, offset);

    u32 shift = (offset & 3) * 8;
    temp &= ~(0xff << shift);
    temp |= data << shift;

    write_pci_config_u32(bdf, offset, temp);
}

void write_pci_config_u16(u32 bdf, unsigned offset, u16 data) {
    u32 temp = read_pci_config_u32(bdf, offset);

    u32 shift = (offset & 2) * 8;
    temp &= ~(0xffff << shift);
    temp |= data << shift;

    write_pci_config_u32(bdf, offset, temp);
}

void write_pci_config_u32(u32 bdf, unsigned offset, u32 data) {
    u32 addr = PCI_ENABLE_BIT | bdf | (offset & 0xfc);
    port_out32(PCI_CONFIG_ADDRESS, addr);
    io_wait();

    port_out32(PCI_CONFIG_DATA, data);
    io_wait();
}

int is_pci_bridge(struct pci_device *dev) {
    return dev->class_code == PCI_BRIDGE_CLASS &&
           dev->subclass == PCI_BRIDGE_SUB_CLASS;
}

static void read_pci_device_config(struct pci_device *dev) {
    u32 bdf = dev->bdf;

    dev->vendor = read_pci_config_u16(bdf, PCI_VENDOR_ID);
    dev->dev = read_pci_config_u16(bdf, PCI_DEVICE_ID);
    dev->command = read_pci_config_u16(bdf, PCI_COMMAND);
    dev->status = read_pci_config_u16(bdf, PCI_STATUS);
    dev->prog_if = read_pci_config_u8(bdf, PCI_PROG_IF);
    dev->subclass = read_pci_config_u8(bdf, PCI_SUBCLASS);
    dev->class_code = read_pci_config_u8(bdf, PCI_CLASS);
    dev->hdr_type = read_pci_config_u8(bdf, PCI_HEADER_TYPE);

    dev->interrupt_line = read_pci_config_u16(bdf, PCI_INTERRUPT_LINE);
    dev->interrupt_pin = read_pci_config_u16(bdf, PCI_INTERRUPT_PIN);

    if (is_pci_bridge(dev)) {
        dev->secondary_bus = read_pci_config_u8(dev->bdf, PCI_SECONDARY_BUS);
        dev->subordinate_bus =
            read_pci_config_u8(dev->bdf, PCI_SUBORDINATE_BUS);
    } else {
        dev->sub_vendor_id = read_pci_config_u16(dev->bdf, PCI_SUB_VENDOR);
        dev->subsystem_id = read_pci_config_u16(dev->bdf, PCI_SUB_SYSTEM);
    }
}

static void init_dev(struct pci_device *dev, struct pci_bus *bus, u8 dev_idx,
                     u8 func_idx) {
    u8 bus_idx = bus->index;
    dev->dev_index = dev_idx;
    dev->func_index = func_idx;
    dev->bus = bus;
    dev->bdf = BDF(bus_idx, dev_idx, func_idx);
    read_pci_device_config(dev);
}

static struct pci_bus *scan_bus(u8 bus_idx) {
    struct pci_bus *bus = kzalloc(sizeof(*bus));
    if (bus == NULL)
        return NULL;

    init_list_head(&bus->children);
    init_list_head(&bus->dev_list);
    bus->index = bus_idx;

    for (size_t dev_idx = 0; dev_idx < 32; dev_idx++) {
        for (size_t func_idx = 0; func_idx < 8; func_idx++) {
            u32 bdf = BDF(bus_idx, dev_idx, func_idx);
            u16 vendor_id = read_pci_config_u16(bdf, PCI_VENDOR_ID);
            if (vendor_id == PCI_DEVICE_NOT_EXIST)
                continue;

            struct pci_device *dev = kzalloc(sizeof(*dev));
            if (dev == NULL)
                goto err_free_bus;

            init_dev(dev, bus, dev_idx, func_idx);
            list_add_tail(&dev->list, &bus->dev_list);

            if (is_pci_bridge(dev)) {
                struct pci_bus *sub_bus = scan_bus(dev->secondary_bus);
                if (sub_bus == NULL)
                    goto err_free_bus;

                sub_bus->bridge = dev;
                sub_bus->parent = bus;

                list_add_tail(&sub_bus->list, &bus->children);
            }
        }
    }

    return bus;
    struct pci_device *dev, *temp;
err_free_bus:
    list_for_each_entry_safe(dev, temp, &bus->dev_list, list) {
        kfree(dev);
    }
    kfree(bus);
    return NULL;
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

void release_pci_device(struct pci_device *dev) {
    for (size_t i = 0; i < dev->resource_count; ++i) {
        struct io_resource *res = dev->resources[i];
        switch (res->kind) {
        case IO_RES_PORT:
            release_port_region(res);
            break;
        case IO_RES_MEM:
            release_mem_region(res);
            break;
        }
    }
}

int enable_pci_device(struct pci_device *dev) {
    u8 bars_count = PCI_BARS_COUNT;
    if (is_pci_bridge(dev))
        bars_count = PCI_BRIDGE_BARS_COUNT;

    u8 bus_idx = dev->bus->index;
    u8 dev_idx = dev->dev_index;
    u8 func_idx = dev->func_index;
    u32 bdf = BDF(bus_idx, dev_idx, func_idx);

    dev->resource_count = 0;
    for (u8 bar_idx = 0; bar_idx < bars_count; bar_idx++) {
        u8 bar_offset = PCI_BASE_ADDRESS_0 + bar_idx * sizeof(u32);
        u32 bar = read_pci_config_u32(bdf, bar_offset);

        write_pci_config_u32(bdf, bar_offset, UINT_MAX);
        u32 size = ~read_pci_config_u32(bdf, bar_offset) + 1;
        write_pci_config_u32(bdf, bar_offset, bar);

        if (size == 0)
            continue;

        struct io_resource *res;
        if (bar & 1) {
            // io space bar
            u32 addr = bar & 0xfffffffc;
            res = request_port_region(addr, size);
        } else {
            u8 mem_type = (bar >> 1) & 3;

            u64 addr = bar & 0xfffffff0;
            // 64-bit memory bar
            if (mem_type == 0x2) {
                bar = read_pci_config_u32(bdf, bar_offset + 4);
                addr += (u64)bar << 32;
                bar_idx++;
            }

            res = request_mem_region(addr, size);
        }

        if (!res)
            goto err_free_resources;

        dev->resources[dev->resource_count++] = res;
    }

    return 0;
err_free_resources:
    for (size_t i = 0; i < dev->resource_count; ++i) {
        struct io_resource *res = dev->resources[i];
        switch (res->kind) {
        case IO_RES_PORT:
            release_port_region(res);
            break;
        case IO_RES_MEM:
            release_mem_region(res);
            break;
        }
    }

    return -EBUSY;
}

static struct pci_device *find_pci_dev(struct pci_bus *bus, u16 vendor_id,
                                       u16 dev_id) {
    struct pci_device *dev;
    list_for_each_entry(dev, &bus->dev_list, list) {
        if (dev->vendor == vendor_id && dev->dev == dev_id)
            return dev;
    }

    struct pci_bus *sub_bus;
    list_for_each_entry(sub_bus, &bus->children, list) {
        dev = find_pci_dev(sub_bus, vendor_id, dev_id);
        if (dev)
            return dev;
    }

    return NULL;
}

struct pci_device *get_pci_device(u16 vendor, u16 dev) {
    return find_pci_dev(root_bus, vendor, dev);
}

static void debug_print_dev(struct pci_device *dev) {
    kprintf("DEV: bus=%d, dev=%d, func=%d, vendor=%x, devid=%x, "
            "class=%#x, subclass=%#x\n",
            dev->bus->index, dev->dev_index, dev->func_index, dev->vendor,
            dev->dev, dev->class_code, dev->subclass);
}

void debug_print_bus(struct pci_bus *bus) {
    kprintf("BUS: bus=%d\n", bus->index);

    struct pci_device *dev;
    list_for_each_entry(dev, &bus->dev_list, list) {
        debug_print_dev(dev);
    }

    struct pci_bus *sub_bus = NULL;
    list_for_each_entry(sub_bus, &bus->children, list) {
        debug_print_bus(sub_bus);
    }
}
