#pragma once

#include <moose/types.h>

struct stat;
struct timeval;
struct timezone;

u64 __syscall(u64 function, u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4);

int open(const char *name, int flags, mode_t mode);
int creat(const char *name, mode_t mode);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);
int stat(int fd, struct stat *stat);
int fork(void);
int execve(const char *name, const char **argv, const char **envp);
int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(const struct timeval *tv, const struct timezone *tz);
int mount(const char *source, const char *dst, const char *fstype,
          unsigned long mountflags, const void *data);
int umount(const char *target);
int mkdir(const char *name, mode_t mode);
int rmdir(const char *name);
int unlink(const char *name);
int link(const char *oldpath, const char *newpath);
int rename(const char *oldpath, const char *newpath);
int chmod(const char *filename, mode_t mode);
int fchmod(int fd, mode_t mode);
off_t lseek(int fd, off_t offset, int whence);
mode_t umask(mode_t mask);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int fcntl(int fd, int cmd, ...);
int ioctl(int fd, int cmd, ...);
void *sbrk(intptr_t increment);
