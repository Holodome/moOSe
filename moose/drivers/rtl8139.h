#pragma once

#include <net/frame.h>
#include <types.h>

#define RTL8139_VENDOR_ID 0x10ec
#define RTL8139_DEVICE_ID 0x8139

int init_rtl8139(void);
void rtl8139_get_mac(u8 mac[static 6]);
void destroy_rtl8139(void);

void rtl8139_send(const void *frame, size_t size);
