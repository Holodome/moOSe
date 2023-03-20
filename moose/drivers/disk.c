#include <blk_device.h>
#include <drivers/ata.h>
#include <drivers/disk.h>
#include <mbr.h>
#include <string.h>

static struct blk_device disk_dev_;
struct blk_device *disk_dev = &disk_dev_;
static struct blk_device disk_part_dev_;
struct blk_device *disk_part_dev = &disk_part_dev_;
static struct blk_device disk_part1_dev_;
struct blk_device *disk_part1_dev = &disk_part1_dev_;

static u32 partition_start;
static u32 partition1_start;

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

static int partition1_read_block(struct blk_device *dev __unused, size_t idx,
                                 void *buf) {
    return ata_read_block(idx + partition1_start, buf);
}

static int partition1_write_block(struct blk_device *dev __unused, size_t idx,
                                  const void *buf) {
    return ata_write_block(idx + partition1_start, buf);
}

void print_blk_device(struct blk_device *dev) {
    kprintf("blk_dev %s, capacity=%lu, block_size=%lu\n", dev->name,
            (long unsigned)dev->capacity, (long unsigned)dev->block_size);
}

void init_disk(void) {
    strlcpy(disk_dev->name, "sda", sizeof(disk_dev->name));
    disk_dev->block_size = 512;
    disk_dev->block_size_log = 9;
    disk_dev->read_block = disk_read_block;
    disk_dev->write_block = disk_write_block;
    init_blk_device(disk_dev);

    struct mbr_partition partition;
    blk_read(disk_dev, MBR_PARTITION_OFFSET, &partition, sizeof(partition));
    partition_start = partition.addr;

    strlcpy(disk_part_dev->name, "sda1", sizeof(disk_dev->name));
    disk_part_dev->block_size = 512;
    disk_part_dev->block_size_log = 9;
    disk_part_dev->read_block = partition_read_block;
    disk_part_dev->write_block = partition_write_block;
    disk_part_dev->capacity = partition.size;
    init_blk_device(disk_part_dev);

    blk_read(disk_dev, MBR_PARTITION_OFFSET + MBR_PARTITION_SIZE, &partition,
             sizeof(partition));
    partition1_start = partition.addr;

    strlcpy(disk_part1_dev->name, "sda2", sizeof(disk_dev->name));
    disk_part1_dev->block_size = 512;
    disk_part1_dev->block_size_log = 9;
    disk_part1_dev->read_block = partition1_read_block;
    disk_part1_dev->write_block = partition1_write_block;
    disk_part1_dev->capacity = partition.size;
    init_blk_device(disk_part1_dev);
}

