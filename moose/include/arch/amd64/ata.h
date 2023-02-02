#pragma once

#include <types.h>

enum ata_dev_kind {
    ATA_DEV_PATA,
    ATA_DEV_SATA,
    ATA_DEV_PATAPI,
    ATA_DEV_SATAPI
};

#define ATA_PIO_BLKSZ 512
int ata_pio_read(void *buf, u32 lba, u8 sector_count);
