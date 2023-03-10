// Note that we use amd64 files here with amd64 types.
// But it turns out that definitions for basic types (u32) are the same.
// The only difference is in the size of pointer, but we can work around that
// (usize sized types) and it makes no difference
#include <../arch/amd64/ata.c>
#include <../device.c>
#include <../disk.c>
#include <../fs/fat.c>
#include <../mm/kmalloc.c>
#include <../string.c>
#include <../ctype.c>

#include <mbr.h>

extern void print(const char *s);
void __panic(void) { __builtin_unreachable(); }
int kprintf(const char *fmt __attribute__((unused)), ...) { return 0; }
void *vsbrk(intptr_t inc __attribute__((unused))) { return NULL; }

int load_kernel(void) {
    init_kmalloc();
    int result = init_disk();
    if (result)
        return result;

    struct fatfs fs = {.dev = disk_part_dev};

    result = fatfs_mount(&fs);
    if (result != 0) {
        print("failed to mount");
        return result;
    }

    struct fatfs_file file;
    result = fatfs_open(&fs, "kernel.bin", &file);
    if (result != 0) {
        print("failed to open");
        return result;
    }

    uintptr_t addr = 0x100000;
    u32 iterations = (file.size + 511) / 512;
    for (; iterations--; addr += 512) {
        result = fatfs_read(&fs, &file, (void *)addr, 512);
        if (result < 0) {
            print("failed to read");
            return result;
        }
    }

    return 0;
}
