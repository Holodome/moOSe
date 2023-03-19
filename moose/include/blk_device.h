#pragma once

#include <fs/vfs.h>
#include <types.h>

#define BLK_DEVICE_NAME_LEN 32

struct blk_device {
    char name[BLK_DEVICE_NAME_LEN];
    blkcnt_t capacity;
    blcksize_t block_size;
    u8 block_size_log;

    void *private;

    int (*read_block)(struct blk_device *dev, size_t idx, void *buf);
    int (*write_block)(struct blk_device *dev, size_t idx, const void *buf);
};

void blk_read(struct blk_device *dev, size_t at, void *buf, size_t size);
void blk_write(struct blk_device *dev, size_t at, const void *buf, size_t size);
int init_blk_device(struct blk_device *dev);
