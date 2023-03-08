#pragma once

#include <list.h>
#include <mm/kmem.h>
#include <mm/resource.h>

#define PCI_BARS_COUNT 6

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

#define BDF(bus, device, func) \
    (((bus) << 16) | ((device) << 11) | ((func) << 8))

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
    u32 bdf;
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

    struct resource *resources[PCI_BARS_COUNT];
};

int init_pci(void);
int enable_pci_device(struct pci_device *device);
struct pci_bus *get_root_bus(void);
struct pci_device *get_pci_device(u16 vendor, u16 device);

// bdf - (bus, device, function) encoded in u32
u32 read_pci_config_u32(u32 bdf, u8 offset);
u16 read_pci_config_u16(u32 bdf, u8 offset);
u8 read_pci_config_u8(u32 bdf, u8 offset);

void write_pci_config_u32(u32 bdf, u8 offset, u32 data);
void write_pci_config_u16(u32 bdf, u8 offset, u16 data);
void write_pci_config_u8(u32 bdf, u8 offset, u8 data);

int is_pci_bridge(struct pci_device *device);

void debug_print_bus(struct pci_bus *bus);
