#pragma once

#include <moose/fs/posix.h>
#include <moose/types.h>

#define __ENUMERATE_SYSCALLS                                                   \
    SYSCALL(open)                                                              \
    SYSCALL(creat)                                                             \
    SYSCALL(read)                                                              \
    SYSCALL(write)                                                             \
    SYSCALL(close)                                                             \
    SYSCALL(stat)                                                              \
    SYSCALL(fork)                                                              \
    SYSCALL(execve)                                                            \
    SYSCALL(gettimeofday)                                                      \
    SYSCALL(settimeofday)                                                      \
    SYSCALL(mount)                                                             \
    SYSCALL(umount)                                                            \
    SYSCALL(mkdir)                                                             \
    SYSCALL(rmdir)                                                             \
    SYSCALL(unlink)                                                            \
    SYSCALL(link)                                                              \
    SYSCALL(rename)                                                            \
    SYSCALL(chmod)                                                             \
    SYSCALL(fchmod)                                                            \
    SYSCALL(lseek)                                                             \
    SYSCALL(umask)                                                             \
    SYSCALL(dup)                                                               \
    SYSCALL(dup2)                                                              \
    SYSCALL(fcntl)                                                             \
    SYSCALL(ioctl)                                                             \
    SYSCALL(sbrk)                                                              \
    SYSCALL(exit)                                                              \
    SYSCALL(wait)                                                              \
    SYSCALL(kill)                                                              \
    SYSCALL(readlink)                                                          \
    SYSCALL(fstat)

enum {
#define SYSCALL(_name) __SYS_##_name,
    __ENUMERATE_SYSCALLS
#undef SYSCALL
};

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct timeval {
    time_t vs_sec;
    suseconds_t tv_usec;
};

struct timezone {
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime;     /* type of DST correction */
};

struct stat {
    dev_t st_dev;         /* ID of device containing file */
    ino_t st_ino;         /* Inode number */
    mode_t st_mode;       /* File type and mode */
    nlink_t st_nlink;     /* Number of hard links */
    uid_t st_uid;         /* User ID of owner */
    gid_t st_gid;         /* Group ID of owner */
    dev_t st_rdev;        /* Device ID (if special file) */
    off_t st_size;        /* Total size, in bytes */
    blksize_t st_blksize; /* Block size for filesystem I/O */
    blkcnt_t st_blocks;   /* Number of 512B blocks allocated */

    /* Since Linux 2.6, the kernel supports nanosecond
       precision for the following timestamp fields.
       For the details before Linux 2.6, see NOTES. */

    struct timespec st_atim; /* Time of last access */
    struct timespec st_mtim; /* Time of last modification */
    struct timespec st_ctim; /* Time of last status change */
};

struct registers_state;

struct syscall_parameters {
    u64 function;
    u64 arg0;
    u64 arg1;
    u64 arg2;
    u64 arg3;
    u64 arg4;
};
void parse_syscall_parameters(const struct registers_state *state,
                              struct syscall_parameters *params);
void set_syscall_result(u64 result, struct registers_state *state);

void syscall_handler(struct registers_state *state);

int sys$open(const char *name, int flags, mode_t mode);
int sys$creat(const char *name, mode_t mode);
ssize_t sys$read(int fd, void *buf, size_t count);
ssize_t sys$write(int fd, void *buf, size_t count);
int sys$close(int fd);
int sys$stat(int fd, struct stat *stat);
int sys$fork(void);
int sys$execve(const char *name, const char **argv, const char **envp);
int sys$gettimeofday(struct timeval *tv, struct timezone *tz);
int sys$settimeofday(const struct timeval *tv, const struct timezone *tz);
int sys$mount(const char *source, const char *dst, const char *fstype,
              unsigned long mountflags, const void *data);
int sys$umount(const char *target);
int sys$mkdir(const char *name, mode_t mode);
int sys$rmdir(const char *name);
int sys$unlink(const char *name);
int sys$link(const char *oldpath, const char *newpath);
int sys$rename(const char *oldpath, const char *newpath);
int sys$chmod(const char *filename, mode_t mode);
int sys$fchmod(int fd, mode_t mode);
off_t sys$lseek(int fd, off_t offset, int whence);
mode_t sys$umask(mode_t mask);
int sys$dup(int oldfd);
int sys$dup2(int oldfd, int newfd);
int sys$fcntl(int fd, int cmd, unsigned long arg);
int sys$ioctl(int fd, int cmd, unsigned long arg);
void *sys$sbrk(intptr_t increment);
__noreturn void sys$exit(int status);
int sys$kill(pid_t pid, int sig);
int sys$wait(int *wstatus);
ssize_t sys$readlink(const char *pathname, char *buf, size_t bufsiz);
int sys$fstat(int fd, struct stat *stat);
