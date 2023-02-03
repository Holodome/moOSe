#pragma once

#include <types.h>

struct device;

struct blk_device {
    size_t block_size;
    u8 block_size_log;

    int (*read_block)(struct device *dev, size_t idx, void *buf);
    int (*write_block)(struct device *dev, size_t idx, const void *buf);
};

struct file_operations {
    off_t (*lseek)(struct device *dev, off_t off, int whence);
    ssize_t (*read)(struct device *dev, void *buf, size_t buf_size);
    ssize_t (*write)(struct device *dev, const void *buf, size_t buf_size);
    int (*flush)(struct device *dev);
};

struct device {
    const char *name;
    void *private_data;
    struct file_operations ops;
};

int init_blk_device(struct blk_device *blk, struct device *dev);

off_t lseek(struct device *dev, off_t off, int whence);
ssize_t read(struct device *dev, void *buf, size_t buf_size);
ssize_t write(struct device *dev, const void *buf, size_t buf_size);
int flush(struct device *dev);
