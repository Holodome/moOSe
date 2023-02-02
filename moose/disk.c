#include <arch/amd64/ata.h>
#include <disk.h>
#include <errno.h>
#include <fs/fat.h>
#include <kmem.h>
#include <mbr.h>

ssize_t disk_read(void *buf, size_t size);
ssize_t disk_write(const void *buf, size_t size);
ssize_t disk_seek(off_t off, int whence);

static struct {
    u32 pos;
    u8 block[512];
    u32 current_block;
    u32 partition_start;
    u32 partition_size;
} cursor = {.current_block = 0xffffffff};

int disk_init(void) {
    struct mbr_partition part_info;
    int result = disk_seek(MBR_PARTITION_OFFSET, SEEK_SET);
    if (result != 0) {
        return -1;
    }

    result = disk_read(&part_info, sizeof(part_info));
    if (result != sizeof(part_info)) {
        return -1;
    }

    cursor.partition_start = part_info.addr * 512;
    cursor.partition_size = part_info.size * 512;

    return 0;
}

ssize_t disk_read(void *buf, size_t size) {
    char *dst = buf;
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

        memcpy(dst, cursor.block + offset, to_copy);
        cursor.pos += to_copy;
        size -= to_copy;
        dst += to_copy;
    }

    return dst - (char *)buf;
}

ssize_t disk_write(const void *buf, size_t size) {
    const char *src = buf;
    size_t total_wrote = 0;
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

        memcpy(cursor.block + offset, src, to_copy);
        cursor.pos += to_copy;
        size -= to_copy;
        total_wrote += to_copy;

        if (ata_pio_write(cursor.block, lba, 1)) {
            return -EIO;
        }
    }

    return total_wrote;
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

ssize_t disk_partition_read(void *buf, size_t size) {
    return disk_read(buf, size);
}

ssize_t disk_partition_write(const void *buf, size_t size) {
    return disk_write(buf, size);
}

ssize_t disk_partition_seek(off_t off, int whence) {
    if (whence == SEEK_SET) {
        off += cursor.partition_start;
    }
    return disk_seek(off, whence);
}
