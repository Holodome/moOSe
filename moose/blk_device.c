#include <moose/blk_device.h>
#include <moose/errno.h>
#include <moose/kstdio.h>
#include <moose/mm/kmalloc.h>
#include <moose/panic.h>
#include <moose/string.h>

struct blk_device_buffered {
    struct blk_device *dev;
    u32 pos;
    u32 current_block;
    char *buffer;
};

static off_t buffered_lseek(struct blk_device *dev, off_t off, int whence) {
    struct blk_device_buffered *buf = dev->private;
    switch (whence) {
    case SEEK_CUR:
        buf->pos += off;
        break;
    case SEEK_SET:
        buf->pos = off;
        break;
    case SEEK_END:
    default:
        return -EINVAL;
    }

    return 0;
}

static ssize_t buffered_read(struct blk_device *dev, void *dst_, size_t size) {
    struct blk_device_buffered *buf = dev->private;
    char *dst = dst_;
    while (size) {
        u32 lba = buf->pos / dev->block_size;
        u32 offset = buf->pos % dev->block_size;

        if (buf->current_block != lba) {
            if (dev->read_block(dev, lba, buf->buffer))
                return -EIO;
            buf->current_block = lba;
        }

        size_t to_copy = size;
        if (offset + to_copy > dev->block_size)
            to_copy = dev->block_size - offset;

        memcpy(dst, buf->buffer + offset, to_copy);
        buf->pos += to_copy;
        size -= to_copy;
        dst += to_copy;
    }

    return dst - (char *)dst_;
}

static ssize_t buffered_write(struct blk_device *dev, const void *src_,
                              size_t size) {
    struct blk_device_buffered *buf = dev->private;
    const char *src = src_;
    size_t total_wrote = 0;
    while (size) {
        u32 lba = buf->pos / dev->block_size;
        u32 offset = buf->pos % dev->block_size;
        if (buf->current_block != lba) {
            if (dev->read_block(dev, lba, buf->buffer))
                return -EIO;
            buf->current_block = lba;
        }

        size_t to_copy = size;
        if (offset + to_copy > dev->block_size)
            to_copy = dev->block_size - offset;

        memcpy(buf->buffer + offset, src, to_copy);
        buf->pos += to_copy;
        size -= to_copy;
        total_wrote += to_copy;

        if (dev->write_block(dev, lba, buf->buffer))
            return -EIO;
    }

    return total_wrote;
}

int init_blk_device(struct blk_device *blk) {
    struct blk_device_buffered *block =
        kzalloc(sizeof(*block) + blk->block_size);
    if (block == NULL)
        return -1;

    block->buffer = (void *)(block + 1);
    block->dev = blk;
    block->current_block = -1;
    blk->private = block;

    return 0;
}

void blk_read(struct blk_device *dev, size_t at, void *buf, size_t size) {
    buffered_lseek(dev, at, SEEK_SET);
    if (buffered_read(dev, buf, size) < 0)
        panic("blk_read failed");
}

void blk_write(struct blk_device *dev, size_t at, const void *buf,
               size_t size) {
    buffered_lseek(dev, at, SEEK_SET);
    if (buffered_write(dev, buf, size) < 0)
        panic("blk_write failed");
}

void print_blk_device(struct blk_device *dev) {
    kprintf("blk_dev %s capacity=%lu block_size=%lu\n", dev->name,
            (long unsigned)dev->capacity, (long unsigned)dev->block_size);
}
