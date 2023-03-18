#pragma once

#include <fs/vfs.h>
#include <types.h>

struct device;

struct blk_device {
    size_t block_size;
    u8 block_size_log;

    int (*read_block)(struct device *dev, size_t idx, void *buf);
    int (*write_block)(struct device *dev, size_t idx, const void *buf);
};

void blk_read(struct blk_device *dev, size_t at, void *buf, size_t size);
void blk_write(struct blk_device *dev, size_t at, const void *buf, size_t size);

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

int seek_read(struct device *dev, off_t off, void *buf, size_t buf_size);
int seek_write(struct device *dev, off_t off, const void *buf, size_t buf_size);
