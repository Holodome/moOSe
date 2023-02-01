#include <arch/amd64/ata.h>
#include <arch/amd64/idt.h>
#include <arch/amd64/keyboard.h>
#include <arch/amd64/memory_map.h>
#include <kmem.h>
#include <kstdio.h>
#include <tty.h>
#include <errno.h>

#include <fs/fat.h>

struct fatfs_cursor {
    struct pfatfs *fs;
    u32 pos;

    u8 block[512];
    u32 current_block;
    u32 start;
};

static ssize_t read(void *handle, void *buf, size_t size) {
    struct fatfs_cursor *cursor = handle;

    u32 lba = cursor->pos / 512 + cursor->start;
    u32 offset = cursor->pos % 512;
    if (cursor->current_block != lba) {
        if (ata_pio_read(cursor->block, lba, 1)) {
            kprintf("read %u (%#x) failed\n", lba, lba * 512);
            return -1;
        }
        cursor->current_block = lba;
    }

    memcpy(buf, cursor->block + offset, size);
    cursor->pos += size;
    return size;
}

static ssize_t write(void *handle, const void *buf, size_t size) {
    (void)handle;
    (void)buf;
    (void)size;
    return -1;
}

static ssize_t seek(void *handle, i32 off, int whence) {
    struct fatfs_cursor *cursor = handle;
    switch (whence) {
    case SEEK_CUR:
        cursor->pos = cursor->pos + off;
        break;
    case SEEK_END:
        return -1;
    case SEEK_SET:
        cursor->pos = off;
        break;
    default:
        return -1;
    }

    return 0;
}

__attribute__((unused)) static void test_fs(void) {
    struct pfatfs_settings settings = {
        .read = read, .write = write, .seek = seek};
    struct pfatfs fs = {.settings = &settings};
    struct fatfs_cursor cursor = {.fs = &fs, .start = 0x7d600 / 512};
    settings.handle = &cursor;

    int r = pfatfs_mount(&fs);
    if (r != 0) {
        kprintf("e: %d\n", r);
        for (;;)
            ;
    }

    pfatfs_file file = {0};
    r = pfatfs_open(&fs, "/", &file);
    if (r != 0) {
        kprintf("open e: %d\n", r);
        for (;;)
            ;
    }

    size_t i = 0;
    for (;; ++i) {
        pfatfs_file new_file = {0};
        r = pfatfs_readdir(&fs, &file, &new_file);
        if (r == -ENOENT) {
            break;
        }
        if (r < 0) {
            kprintf("e: %d\n", r);
            for (;;)
                ;
        }
        kprintf("%u: %11s: %u\n", i, (char *)new_file.name, new_file.size);
    }
}

__attribute__((noreturn)) void kmain(void) {
    kputs("running moOSe kernel");
    kprintf("build %s %s\n", __DATE__, __TIME__);

    const struct memmap_entry *memmap;
    u32 memmap_size;
    get_memmap(&memmap, &memmap_size);
    kprintf("Bios-provided physical RAM map:\n");
    for (u32 i = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        kprintf("%#016llx-%#016llx: %s(%u)\n", (unsigned long long)entry->base,
                (unsigned long long)(entry->base + entry->length),
                get_memmap_type_str(entry->type), (unsigned)entry->type);
    }

    setup_idt();
    init_keyboard();

    kprintf(">");
    char buffer[32] = {0};
    ssize_t len = tty_read(buffer, sizeof(buffer));
    buffer[len] = 0;
    kprintf("received %s (%d)\n", buffer, (int)len);

    for (;;)
        ;
}
