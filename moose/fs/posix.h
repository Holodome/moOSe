#pragma once

#include <moose/time.h>
#include <moose/types.h>

// <stdio.h>

#define SEEK_SET 0
#define SEEK_END 1
#define SEEK_CUR 2

// <moose/sys/stat.h>

// note: these are shamelessly taken from linux

#define S_IFMT 00170000
#define S_IFSOCK 0140000
#define S_IFLNK 0120000
#define S_IFREG 0100000
#define S_IFBLK 0060000
#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFIFO 0010000
#define S_ISUID 0004000
#define S_ISGID 0002000
#define S_ISVTX 0001000

#define S_ISLNK(m) (((m)&S_IFMT) == S_IFLNK)
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#define S_ISCHR(m) (((m)&S_IFMT) == S_IFCHR)
#define S_ISBLK(m) (((m)&S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m)&S_IFMT) == S_IFIFO)
#define S_ISSOCK(m) (((m)&S_IFMT) == S_IFSOCK)

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

struct kstat {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blkcnt;

    struct ktimespec st_atim;
    struct ktimespec st_mtim;
    struct ktimespec st_ctim;
};

// <fcntl.h>

#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4
#define F_GETLK 5
#define F_SETLK 6
#define F_SETLKW 7
#define F_SETOWN 8
#define F_GETOWN 9

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_RDWR 00000002
#define O_ACCMODE 00000003

#define O_CREAT 00000004
#define O_EXCL 00000010
#define O_NOCTTY 00000020
#define O_TRUNC 00000040

#define O_APPEND 00000100
#define O_DSYNC O_SYNC // We do not implement O_DSYNC!
#define O_NONBLOCK 0000400
#define O_RSYNC O_SYNC // We do not implement O_RSYNC!
#define O_SYNC 00010000
