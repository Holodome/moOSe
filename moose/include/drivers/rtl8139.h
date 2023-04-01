#pragma once

#include <net/device.h>
#include <net/frame.h>
#include <types.h>

#define RTL8139_VENDOR_ID 0x10ec
#define RTL8139_DEVICE_ID 0x8139

struct net_device *create_rtl8139(void);
void destroy_rtl8139(struct net_device *dev);
