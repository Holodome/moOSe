#include <../arch/amd64/ata.c>
#include <../disk.c>
#include <../fs/fat.c>

struct fatfs_cursor {
    struct pfatfs *fs;
    u32 pos;

    u8 block[512];
    u32 current_block;
    u32 start;
};

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
        off += 9216 + 512;
    }
    return disk_seek(off, whence);
}

extern void print(const char *s);

int load_kernel(void) {
    struct pfatfs fs = {.settings = &(struct pfatfs_settings){
                            .seek = seek, .read = read, .write = write}};
    int result = pfatfs_mount(&fs);
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

    u32 addr = 0x100000;
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
