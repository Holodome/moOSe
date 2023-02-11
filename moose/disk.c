#include <arch/amd64/ata.h>
#include <disk.h>
#include <errno.h>
#include <fs/fat.h>
#include <kmem.h>
#include <mbr.h>

static struct device disk_dev_;
static struct device disk_part_dev_;
struct device *disk_dev = &disk_dev_;
struct device *disk_part_dev = &disk_part_dev_;

static u32 partition_start;

static int read_partition_info(void) {
    if (lseek(disk_dev, MBR_PARTITION_OFFSET, SEEK_SET) < 0)
        return -1;

    struct mbr_partition partition;
    ssize_t read_result = read(disk_dev, &partition, sizeof(partition));
    if (read_result < 0 || (size_t)read_result != sizeof(partition))
        return -1;

    partition_start = partition.addr;
    return 0;
}

static int partition_read_block(struct device *dev __attribute__((unused)),
                                size_t idx, void *buf) {
    return ata_pio_dev->read_block(dev, idx + partition_start, buf);
}

static int partition_write_block(struct device *dev __attribute__((unused)),
                                 size_t idx, const void *buf) {
    return ata_pio_dev->write_block(dev, idx + partition_start, buf);
}

int disk_init(void) {
    struct blk_device *blk_dev = ata_pio_dev;
    if (init_blk_device(blk_dev, disk_dev))
        return -1;

    if (read_partition_info())
        return -1;

    static struct blk_device partition_blk = {
        .read_block = partition_read_block,
        .write_block = partition_write_block};
    partition_blk.block_size_log = blk_dev->block_size_log;
    partition_blk.block_size = blk_dev->block_size;

    if (init_blk_device(&partition_blk, disk_part_dev))
        return -1;

    return 0;
}
