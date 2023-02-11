#pragma once

#include <types.h>

typedef enum { PFATFS_FAT12, PFATFS_FAT16, PFATFS_FAT32 } pfatfs_kind;

typedef enum { PFATFS_FILE_REG, PFATFS_FILE_DIR } pfatfs_file_type;

// Archive
#define PFATFS_FATTR_ARCH 0x1
// Read-only
#define PFATFS_FATTR_RO 0x2
// System
#define PFATFS_FATTR_SYS 0x4
// Hidden file
#define PFATFS_FATTR_HID 0x8

typedef struct pfatfs {
    struct device *device;
    pfatfs_kind kind;

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
} pfatfs;

typedef struct pfatfs_file {
    char name[11];
    pfatfs_file_type type;
    u32 dirent_loc;
    u8 attrs;

    u16 offset;
    u16 cluster_offset;
    u32 cluster;
    u32 start_cluster;
    u32 size;
} pfatfs_file;

typedef struct pfatfs_date {
    u8 day;
    u8 month;
    // starting from 1980
    u8 year;
} pfatfs_date;

typedef struct pfatfs_time {
    u8 secs;
    u8 mins;
    u8 hours;
} pfatfs_time;

typedef struct pfatfs_file_create_info {
    u8 attrs;
    pfatfs_date date;
    pfatfs_time time;
} pfatfs_file_create_info;

int pfatfs_mount(pfatfs *fs);

ssize_t pfatfs_read(pfatfs *fs, pfatfs_file *file, void *buffer, size_t count);
ssize_t pfatfs_write(pfatfs *fs, pfatfs_file *file, const void *buffer,
                     size_t count);
int pfatfs_seek(pfatfs *fs, pfatfs_file *file, off_t offset, int whence);
int pfatfs_truncate(pfatfs *fs, pfatfs_file *file, size_t length);
int pfatfs_readdir(pfatfs *fs, pfatfs_file *file, pfatfs_file *child);

int pfatfs_open(pfatfs *fs, const char *filename, pfatfs_file *file);
int pfatfs_create(pfatfs *fs, const char *filename,
                  const pfatfs_file_create_info *info, pfatfs_file *file);
int pfatfs_rename(pfatfs *fs, const char *oldpath, const char *newpath);
int pfatfs_mkdir(pfatfs *fs, const char *path);
int pfatfs_remove(pfatfs *fs, const char *path);
