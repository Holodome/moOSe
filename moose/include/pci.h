#pragma once

#include <list.h>
#include <mm/kmem.h>

#define PCI_BARS_COUNT 6

struct pci_device;

struct pci_bus {
    u8 index;
    struct list_head list;
    struct pci_bus *parent;
    struct list_head children;

    struct pci_device *bridge;
    struct list_head devices;
};

// header type 0x2 (PCI-to-CardBus bridge) is unsupported
struct pci_device {
    struct list_head list;
    struct pci_bus *bus;

    u8 device_index;
    u8 func_index;
    u16 vendor;
    u16 device;
    u16 command;
    u16 status;
    u8 prog_if;
    u8 subclass;
    u8 class_code;
    u8 hdr_type;

    // for header type 0x0
    u16 sub_vendor_id;
    u16 subsystem_id;

    // for header type 0x1 (pci-to-pci bridge)
    u8 secondary_bus;
    u8 subordinate_bus;

    u8 interrupt_line;
    u8 interrupt_pin;

    struct mem_range bars[PCI_BARS_COUNT];
};

void init_pci(void);
struct pci_device *get_pci_device(u16 vendor, u16 device);

void io_wait(void);

u32 read_pci_config_u32(u8 bus, u8 device, u8 function, u8 offset);
u16 read_pci_config_u16(u8 bus, u8 device, u8 function, u8 offset);
u8 read_pci_config_u8(u8 bus, u8 device, u8 function, u8 offset);

void write_pci_config_u32(u8 bus, u8 device, u8 function, u8 offset, u32 data);
void write_pci_config_u16(u8 bus, u8 device, u8 function, u8 offset, u16 data);
void write_pci_config_u8(u8 bus, u8 device, u8 function, u8 offset, u8 data);

int is_pci_bridge(struct pci_device *device);
