#pragma once

#include <fs/vfs.h>
#include <types.h>

enum fatfs_kind {
    PFATFS_FAT12,
    PFATFS_FAT16,
    PFATFS_FAT32
};

enum fatfs_file_type {
    FATFS_FILE_REG,
    FATFS_FILE_DIR
};

// Archive
#define PFATFS_FATTR_ARCH 0x1
// Read-only
#define PFATFS_FATTR_RO 0x2
// System
#define PFATFS_FATTR_SYS 0x4
// Hidden file
#define PFATFS_FATTR_HID 0x8

struct fatfs {
    struct blk_device *dev;
    enum fatfs_kind kind;

    union {
        u32 cluster;
        // on legacy fat systems rootdir is stored as sequence of sectors
        // instead of regular file
        struct {
            u32 offset;
            u32 size;
        } legacy;
    } rootdir;
    u32 fat_offset;
    u32 data_offset;
    u16 bytes_per_cluster;
    u32 cluster_count;
};

struct fatfs_file {
    char name[11];
    enum fatfs_file_type type;
    u32 dirent_loc;
    u8 attrs;

    u16 offset;
    u16 cluster_offset;
    u32 cluster;
    u32 start_cluster;
    u32 size;
};

struct fatfs_date {
    u8 day;
    u8 month;
    // starting from 1980
    u8 year;
};

struct fatfs_time {
    u8 secs;
    u8 mins;
    u8 hours;
};

struct fatfs_file_create_info {
    u8 attrs;
    struct fatfs_date date;
    struct fatfs_time time;
};

int fatfs_mount(struct fatfs *fs);

ssize_t fatfs_read(struct fatfs *fs, struct fatfs_file *file, void *buffer,
                   size_t count);
ssize_t fatfs_write(struct fatfs *fs, struct fatfs_file *file,
                    const void *buffer, size_t count);
int fatfs_seek(struct fatfs *fs, struct fatfs_file *file, off_t offset,
               int whence);
int fatfs_truncate(struct fatfs *fs, struct fatfs_file *file, size_t length);
int fatfs_readdir(struct fatfs *fs, struct fatfs_file *file,
                  struct fatfs_file *child);

int fatfs_open(struct fatfs *fs, const char *filename, struct fatfs_file *file);
int fatfs_create(struct fatfs *fs, const char *filename,
                 const struct fatfs_file_create_info *info,
                 struct fatfs_file *file);
int fatfs_rename(struct fatfs *fs, const char *oldpath, const char *newpath);
int fatfs_mkdir(struct fatfs *fs, const char *path);
int fatfs_remove(struct fatfs *fs, const char *path);
