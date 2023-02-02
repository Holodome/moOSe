#include <arch/amd64/ata.h>
#include <disk.h>
#include <errno.h>
#include <fs/fat.h>
#include <kmem.h>

ssize_t disk_read(void *buf, size_t size);
ssize_t disk_write(const void *buf, size_t size);
ssize_t disk_seek(off_t off, int whence);

static struct {
    u32 pos;
    u8 block[512];
    u32 current_block;
} cursor = {};

ssize_t disk_read(void *buf, size_t size) {
    while (size) {
        u32 lba = cursor.pos / 512;
        u32 offset = cursor.pos % 512;
        if (cursor.current_block != lba) {
            if (ata_pio_read(cursor.block, lba, 1)) {
                return -EIO;
            }
            cursor.current_block = lba;
        }

        size_t to_copy = size;
        if (offset + to_copy > 512)
            to_copy = 512 - offset;

        memcpy(buf, cursor.block + offset, to_copy);
        cursor.pos += to_copy;
        size -= to_copy;
    }

    return size;
}

ssize_t disk_write(const void *buf, size_t size) {
    (void)buf;
    (void)size;
    return -1;
}

ssize_t disk_seek(off_t off, int whence) {
    switch (whence) {
    case SEEK_CUR:
        cursor.pos = cursor.pos + off;
        break;
    case SEEK_END:
        return -1;
    case SEEK_SET:
        cursor.pos = off;
        break;
    default:
        return -1;
    }

    return 0;
}
