#pragma once

#include <types.h>

enum {
    // No such file or directory
    PFATFS_ENOENT = -1,
    // IO error
    PFATFS_EIO = -2,
    // File expected to be a directory is not
    PFATFS_ENOTDIR = -3,
    // No free space left on disk
    PFATFS_ENOSPC = -4,
    PFATFS_ENAMETOOLONG = -5,
    // Corrupted filesystem
    PFATFS_ECORRUPTED = -6,
    // File expected to be regular file is a directory
    PFATFS_EISDIR = -7,
    // Specified operation can not be executed on root dir
    PFATFS_EROOTDIR = -8,
    // Directory to be deleted is not empty
    PFATFS_ENOTEMPTY = -9,
    PFATFS_EINVAL = -10,
    PFATFS_EEXIST = -11
};

typedef enum { PFATFS_FAT12, PFATFS_FAT16, PFATFS_FAT32 } pfatfs_kind;

typedef enum { PFATFS_FILE_REG, PFATFS_FILE_DIR } pfatfs_file_type;

typedef enum {
    PFATFS_SEEK_SET,
    PFATFS_SEEK_END,
    PFATFS_SEEK_CUR
} pfatfs_whence;

// Archive
#define PFATFS_FATTR_ARCH 0x1
// Read-only
#define PFATFS_FATTR_RO 0x2
// System
#define PFATFS_FATTR_SYS 0x4
// Hidden file
#define PFATFS_FATTR_HID 0x8

typedef struct {
    void *handle;
    ssize_t (*read)(void *handle, void *buf, size_t size);
    ssize_t (*write)(void *handle, const void *buf, size_t size);
    ssize_t (*seek)(void *handle, i32 off, pfatfs_whence whence);
} pfatfs_settings;

typedef struct pfatfs {
    pfatfs_settings *settings;
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
    pfatfs_file_type type;
    u8 attrs;
    pfatfs_date date;
    pfatfs_time time;
} pfatfs_file_create_info;

int pfatfs_mount(pfatfs *fs);

ssize_t pfatfs_read(pfatfs *fs, pfatfs_file *file, void *buffer, size_t count);
ssize_t pfatfs_write(pfatfs *fs, pfatfs_file *file, const void *buffer,
                     size_t count);
int pfatfs_seek(pfatfs *fs, pfatfs_file *file, ssize_t offset,
                pfatfs_whence whence);
int pfatfs_truncate(pfatfs *fs, pfatfs_file *file, size_t length);
int pfatfs_readdir(pfatfs *fs, pfatfs_file *file, pfatfs_file *child);

int pfatfs_openv(pfatfs *fs, const char *filename, size_t filename_len,
                 pfatfs_file *file);
int pfatfs_createv(pfatfs *fs, const char *filename, size_t filename_len,
                   const pfatfs_file_create_info *info, pfatfs_file *file);
int pfatfs_renamev(pfatfs *fs, const char *oldpath, size_t oldpath_len,
                   const char *newpath, size_t newpath_length);
int pfatfs_mkdirv(pfatfs *fs, const char *filepath, size_t filepath_len);
int pfatfs_removev(pfatfs *fs, const char *filepath, size_t filepath_len);

int pfatfs_open(pfatfs *fs, const char *filename, pfatfs_file *file);
int pfatfs_create(pfatfs *fs, const char *filename,
                  const pfatfs_file_create_info *info, pfatfs_file *file);
int pfatfs_rename(pfatfs *fs, const char *oldpath, const char *newpath);
int pfatfs_mkdir(pfatfs *fs, const char *path);
int pfatfs_remove(pfatfs *fs, const char *path);
