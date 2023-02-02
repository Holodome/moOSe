// Note that we use amd64 files here with amd64 types.
// But it turns out that definitions for basic types (u32) are the same.
// The only difference is in the size of pointer, but we can work around that
// (usize sized types) and it makes no difference
#include <../arch/amd64/ata.c>
#include <../disk.c>
#include <../fs/fat.c>

#include <mbr.h>

static u32 start;

static ssize_t read(void *handle __attribute__((unused)), void *buf,
                    size_t size) {
    return disk_read(buf, size);
}

static ssize_t write(void *handle __attribute__((unused)), const void *buf,
                     size_t size) {
    (void)handle;
    (void)buf;
    (void)size;
    return -1;
}

static ssize_t seek(void *handle __attribute__((unused)), off_t off,
                    int whence) {
    if (whence == SEEK_SET) {
        off += start;
    }
    return disk_seek(off, whence);
}

extern void print(const char *s);

int load_kernel(void) {
    struct mbr_partition part_info;
    int result = disk_seek(MBR_PARTITION_OFFSET, SEEK_SET);
    if (result != 0) {
        return result;
    }

    result = disk_read(&part_info, sizeof(part_info));
    if (result != sizeof(part_info)) {
        print("failed to parse mbr");
        return result;
    }

    start = part_info.addr * 512;

    struct pfatfs_settings settings = {0};
    settings.seek = seek;
    settings.read = read;
    settings.write = write;
    struct pfatfs fs = {0};
    fs.settings = &settings;

    result = pfatfs_mount(&fs);
    if (result != 0) {
        print("failed to mount");
        return result;
    }

    struct pfatfs_file file;
    result = pfatfs_open(&fs, "kernel1.bin", &file);
    if (result != 0) {
        print("failed to open");
        return result;
    }

    uintptr_t addr = 0x100000;
    u32 iterations = (file.size + 511) / 512;
    for (; iterations--; addr += 512) {
        result = pfatfs_read(&fs, &file, (void *)addr, 512);
        if (result < 0) {
            print("failed to read");
            return result;
        }
    }

    return 0;
}
