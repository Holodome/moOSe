#include <blk_device.h>
#include <drivers/ata.h>
#include <drivers/disk.h>
#include <mbr.h>

static struct blk_device disk_dev_;
struct blk_device *disk_dev = &disk_dev_;
static struct blk_device disk_part_dev_;
struct blk_device *disk_part_dev = &disk_part_dev_;

static u32 partition_start;

static void read_partition_info(void) {
    struct mbr_partition partition;
    blk_read(disk_dev, MBR_PARTITION_OFFSET, &partition, sizeof(partition));
    partition_start = partition.addr;
}

static int disk_read_block(struct blk_device *dev __unused, size_t idx,
                           void *buf) {
    return ata_read_block(idx, buf);
}

static int disk_write_block(struct blk_device *dev __unused, size_t idx,
                            const void *buf) {
    return ata_write_block(idx, buf);
}

static int partition_read_block(struct blk_device *dev __unused, size_t idx,
                                void *buf) {
    return ata_read_block(idx + partition_start, buf);
}

static int partition_write_block(struct blk_device *dev __unused, size_t idx,
                                 const void *buf) {
    return ata_write_block(idx + partition_start, buf);
}

void init_disk(void) {
    disk_dev->block_size = 512;
    disk_dev->block_size_log = 9;
    disk_dev->read_block = disk_read_block;
    disk_dev->write_block = disk_write_block;
    init_blk_device(disk_dev);
    read_partition_info();

    disk_part_dev->block_size = 512;
    disk_part_dev->block_size_log = 9;
    disk_part_dev->read_block = partition_read_block;
    disk_part_dev->write_block = partition_write_block;
    init_blk_device(disk_part_dev);
}

