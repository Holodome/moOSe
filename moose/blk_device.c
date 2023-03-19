#include <blk_device.h>
#include <kstdio.h>
#include <mm/kmalloc.h>
#include <string.h>

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
    case SEEK_END:
        return -1;
    case SEEK_SET:
        buf->pos = off;
        break;
    default:
        return -1;
    }

    return 0;
}

static ssize_t buffered_read(struct blk_device *dev, void *dst_, size_t size) {
    struct blk_device_buffered *buf = dev->private;
    char *dst = dst_;
    while (size) {
        u32 lba = buf->pos / buf->dev->block_size;
        u32 offset = buf->pos % buf->dev->block_size;

        if (buf->current_block != lba) {
            if (buf->dev->read_block(dev, lba, buf->buffer)) { return -EIO; }
            buf->current_block = lba;
        }

        size_t to_copy = size;
        if (offset + to_copy > 512) to_copy = 512 - offset;

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
        u32 lba = buf->pos / buf->dev->block_size;
        u32 offset = buf->pos % buf->dev->block_size;
        if (buf->current_block != lba) {
            if (buf->dev->read_block(dev, lba, buf->buffer)) { return -EIO; }
            buf->current_block = lba;
        }

        size_t to_copy = size;
        if (offset + to_copy > buf->dev->block_size)
            to_copy = buf->dev->block_size - offset;

        memcpy(buf->buffer + offset, src, to_copy);
        buf->pos += to_copy;
        size -= to_copy;
        total_wrote += to_copy;

        if (buf->dev->write_block(dev, lba, buf->buffer)) { return -EIO; }
    }

    return total_wrote;
}

int init_blk_device(struct blk_device *blk) {
    struct blk_device_buffered *block = kzalloc(sizeof(*block));
    if (block == NULL) return -1;

    block->buffer = kmalloc(blk->block_size);
    if (block->buffer == NULL) {
        kfree(block);
        return -1;
    }

    block->dev = blk;
    block->current_block = -1;
    blk->private = block;

    return 0;
}

void blk_read(struct blk_device *dev, size_t at, void *buf, size_t size) {
    buffered_lseek(dev, at, SEEK_SET);
    buffered_read(dev, buf, size);
}

void blk_write(struct blk_device *dev, size_t at, const void *buf,
               size_t size) {
    buffered_lseek(dev, at, SEEK_SET);
    buffered_write(dev, buf, size);
}
