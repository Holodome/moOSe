#pragma once

#include <device.h>
#include <types.h>

enum ata_dev_kind {
    ATA_DEV_PATA,
    ATA_DEV_SATA,
    ATA_DEV_PATAPI,
    ATA_DEV_SATAPI
};

int ata_read_block(size_t idx, void *buf);
int ata_write_block(size_t idx, const void *buf);
