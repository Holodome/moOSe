#include <arch/amd64/asm.h>
#include <pci.h>
#include <kstdio.h>
#include <mm/kmalloc.h>

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA    0xcfc

// bit 31
#define PCI_ENABLE_BIT 0x80000000

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

void init_pci(void) {
    for (u8 bus = 0; bus != 255; bus++) {
        for (u8 device = 0; device < 32; device++) {
            for (u8 function = 0; function < 8; function++) {
                u16 vendor_id = read_pci_config_u16(bus, device, function, 0x0);
                if (vendor_id != 0xffff) {
                    u16 device_id = read_pci_config_u16(bus, device,
                                                        function, 0x2);
                    kprintf("bus: %u, dev: %u, func: %u, "
                            "vendor_id: %x, device_id: %x\n", bus, device,
                            function, vendor_id, device_id);
                }
            }
        }
    }
}
