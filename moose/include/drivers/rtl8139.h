#pragma once

#include <net/frame.h>
#include <types.h>

#define RTL8139_VENDOR_ID 0x10ec
#define RTL8139_DEVICE_ID 0x8139

int init_rtl8139(u8 *mac_addr);
void rtl8139_send(const void *frame, size_t size);
