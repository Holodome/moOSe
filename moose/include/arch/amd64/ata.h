#pragma once

#include <device.h>
#include <types.h>

enum ata_dev_kind {
    ATA_DEV_PATA,
    ATA_DEV_SATA,
    ATA_DEV_PATAPI,
    ATA_DEV_SATAPI
};

extern struct blk_device *ata_pio_dev;
