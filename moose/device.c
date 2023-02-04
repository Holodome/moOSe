#include <device.h>
#include <errno.h>
#include <kmalloc.h>
#include <kmem.h>
#include <kstdio.h>

off_t lseek(struct device *dev, off_t off, int whence) {
    off_t result = dev->ops.lseek(dev, off, whence);
    int rc = 0;
    if (result < 0) {
        errno = -result;
        rc = -1;
    }

    return rc;
}

ssize_t read(struct device *dev, void *buf, size_t buf_size) {
    ssize_t result = dev->ops.read(dev, buf, buf_size);
    ssize_t rc = result;
    if (result < 0) {
        errno = -result;
        rc = -1;
    }

    return rc;
}

ssize_t write(struct device *dev, const void *buf, size_t buf_size) {
    ssize_t result = dev->ops.write(dev, buf, buf_size);
    ssize_t rc = result;
    if (result < 0) {
        errno = -result;
        rc = -1;
    }

    return rc;
}

int flush(struct device *dev) {
    int rc = 0;
    if (dev->ops.flush)
        rc = dev->ops.flush(dev);

    return rc;
}

struct blk_device_buffered {
    struct blk_device *dev;
    u32 pos;
    u32 current_block;
    char *buffer;
};

static off_t buffered_lseek(struct device *dev, off_t off, int whence) {
    struct blk_device_buffered *buf = dev->private_data;
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

static ssize_t buffered_read(struct device *dev, void *dst_, size_t size) {
    struct blk_device_buffered *buf = dev->private_data;
    char *dst = dst_;
    while (size) {
        u32 lba = buf->pos / buf->dev->block_size;
        u32 offset = buf->pos % buf->dev->block_size;

        if (buf->current_block != lba) {
            if (buf->dev->read_block(dev, lba, buf->buffer)) {
                return -EIO;
            }
            buf->current_block = lba;
        }

        size_t to_copy = size;
        if (offset + to_copy > 512)
            to_copy = 512 - offset;

        memcpy(dst, buf->buffer + offset, to_copy);
        buf->pos += to_copy;
        size -= to_copy;
        dst += to_copy;
    }

    return dst - (char *)dst_;
}

static ssize_t buffered_write(struct device *dev, const void *src_,
                              size_t size) {
    struct blk_device_buffered *buf = dev->private_data;
    const char *src = src_;
    size_t total_wrote = 0;
    while (size) {
        u32 lba = buf->pos / buf->dev->block_size;
        u32 offset = buf->pos % buf->dev->block_size;
        if (buf->current_block != lba) {
            if (buf->dev->read_block(dev, lba, buf->buffer)) {
                return -EIO;
            }
            buf->current_block = lba;
        }

        size_t to_copy = size;
        if (offset + to_copy > buf->dev->block_size)
            to_copy = buf->dev->block_size - offset;

        memcpy(buf->buffer + offset, src, to_copy);
        buf->pos += to_copy;
        size -= to_copy;
        total_wrote += to_copy;

        if (buf->dev->write_block(dev, lba, buf->buffer)) {
            return -EIO;
        }
    }

    return total_wrote;
}

static int buffered_flush(struct device *dev __attribute__((unused))) {
    return 0;
}

static struct file_operations blk_device_buffered_ops = {
    .lseek = buffered_lseek,
    .read = buffered_read,
    .write = buffered_write,
    .flush = buffered_flush,
};

int init_blk_device(struct blk_device *blk, struct device *dev) {
    struct blk_device_buffered *block = kzalloc(sizeof(*block));
    if (block == NULL) {
        return -1;
    }

    block->buffer = kmalloc(blk->block_size);
    if (block->buffer == NULL) {
        kfree(block);
        return -1;
    }

    block->dev = blk;
    block->current_block = -1;
    dev->name = "block device";
    dev->private_data = block;
    memcpy(&dev->ops, &blk_device_buffered_ops, sizeof(dev->ops));

    return 0;
}
