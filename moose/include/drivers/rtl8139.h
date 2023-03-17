#pragma once

#include <types.h>

#define RTL8139_VENDOR_ID 0x10ec
#define RTL8139_DEVICE_ID 0x8139

int init_rtl8139(u8 *mac_addr);
void rtl8139_send(void *frame, u16 size);
void rtl8139_receive(void);
