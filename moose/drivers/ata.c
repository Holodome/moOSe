#include <arch/amd64/asm.h>
#include <drivers/ata.h>

#define PRIMARY_BUS 0x1f0
#define SECONDARY_BUS 0x170

#define DATA_REG 0x0
#define ERR_REG 0x1
#define FEAT_REG 0x1
#define SEC_CNT_REG 0x2
#define SEC_NUM_REG 0x3
#define CYL_LOW_REG 0x4
#define CYL_HIG_REG 0x5
#define DRIVE_HEAD_REG 0x6
#define STAT_CMD_REG 0x7

#define CMD_READ 0x20
#define CMD_WRITE 0x30

static void cache_flush(void) {
    port_out8(PRIMARY_BUS + STAT_CMD_REG, CMD_READ);
    while ((port_in8(PRIMARY_BUS + STAT_CMD_REG) & 0x80) != 0)
        ;
}

static int ata_pio_read(void *buf, u32 lba, u8 sector_count) {
    u16 *cursor = buf;
    port_out8(PRIMARY_BUS + DRIVE_HEAD_REG, 0xe0);
    port_out8(PRIMARY_BUS + FEAT_REG, 0);
    port_out8(PRIMARY_BUS + SEC_CNT_REG, sector_count);
    port_out8(PRIMARY_BUS + SEC_NUM_REG, lba);
    port_out8(PRIMARY_BUS + CYL_LOW_REG, lba >> 8);
    port_out8(PRIMARY_BUS + CYL_HIG_REG, lba >> 16);
    port_out8(PRIMARY_BUS + STAT_CMD_REG, CMD_READ);

    for (int i = 4; i--;) {
        int a = port_in8(PRIMARY_BUS + STAT_CMD_REG);
        if ((a & 0x80) != 0)
            continue;
        if ((a & 0x08) != 0)
            goto read;
    }

    while (sector_count) {
        int a = port_in8(PRIMARY_BUS + STAT_CMD_REG);
        if ((a & 0x80) != 0)
            continue;
        if ((a & 0x21) != 0)
            return -1;
    read:
        for (int i = 256; i--;)
            *cursor++ = port_in16(PRIMARY_BUS + DATA_REG);

        port_in8(PRIMARY_BUS + STAT_CMD_REG);
        port_in8(PRIMARY_BUS + STAT_CMD_REG);
        port_in8(PRIMARY_BUS + STAT_CMD_REG);
        port_in8(PRIMARY_BUS + STAT_CMD_REG);

        --sector_count;
    }

    return 0;
}

static int ata_pio_write(const void *buf, u32 lba, u8 sector_count) {
    const u16 *cursor = buf;
    port_out8(PRIMARY_BUS + DRIVE_HEAD_REG, 0xe0);
    port_out8(PRIMARY_BUS + FEAT_REG, 0);
    port_out8(PRIMARY_BUS + SEC_CNT_REG, sector_count);
    port_out8(PRIMARY_BUS + SEC_NUM_REG, lba);
    port_out8(PRIMARY_BUS + CYL_LOW_REG, lba >> 8);
    port_out8(PRIMARY_BUS + CYL_HIG_REG, lba >> 16);
    port_out8(PRIMARY_BUS + STAT_CMD_REG, CMD_WRITE);

    for (int i = 4; i--;) {
        int a = port_in8(PRIMARY_BUS + STAT_CMD_REG);
        if ((a & 0x80) != 0)
            continue;
        if ((a & 0x08) != 0)
            goto read;
    }

    while (sector_count) {
        int a = port_in8(PRIMARY_BUS + STAT_CMD_REG);
        if ((a & 0x80) != 0)
            continue;
        if ((a & 0x21) != 0)
            return -1;
    read:
        for (int i = 256; i--;) {
            port_out16(PRIMARY_BUS + DATA_REG, *cursor++);
            port_in8(PRIMARY_BUS + STAT_CMD_REG);
        }

        --sector_count;
    }

    cache_flush();

    return 0;
}

int ata_read_block(size_t idx, void *buf) {
    return ata_pio_read(buf, idx, 1);
}

int ata_write_block(size_t idx, const void *buf) {
    return ata_pio_write(buf, idx, 1);
}
