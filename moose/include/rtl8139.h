#pragma once

#include <pci.h>

#define RTL8139_VENDOR_ID 0x10ec
#define RTL8139_DEVICE_ID 0x8139

int init_rtl8139(void);
void debug_print_mac_addr(void);
