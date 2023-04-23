#include <moose/sys/syscalls.h>
#include <moose/sys/usrsys.h>
#include <moose/varargs.h>

u64 __syscall(u64 function, u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4) {
    u64 result;
    asm volatile(
        "movq %[syscall_number], %%rax\n"
        "movq %[arg4], %%r8\n"
        "movq %[arg3], %%r10\n"
        "movq %[arg2], %%rdx\n"
        "movq %[arg1], %%rsi\n"
        "movq %[arg0], %%rdi\n"
        "syscall\n"
        : "=rax"(result)
        : [syscall_number] "g"(function), [arg4] "g"(arg4), [arg3] "g"(arg3),
          [arg2] "g"(arg2), [arg1] "g"(arg1), [arg0] "g"(arg0)
        : "rdi", "rsi", "rdx", "r10", "r8");
    return result;
}

int open(const char *name, int flags, mode_t mode) {
    return (int)__syscall(SYS_OPEN, (u64)name, (u64)flags, (u64)mode, 0, 0);
}

int creat(const char *name, mode_t mode) {
    return (int)__syscall(SYS_CREAT, (u64)name, (u64)mode, 0, 0, 0);
}

ssize_t read(int fd, void *buf, size_t count) {
    return (ssize_t)__syscall(SYS_READ, (u64)fd, (u64)buf, (u64)count, 0, 0);
}

ssize_t write(int fd, void *buf, size_t count) {
    return (ssize_t)__syscall(SYS_READ, (u64)fd, (u64)buf, (u64)count, 0, 0);
}

int close(int fd) {
    return (int)__syscall(SYS_CLOSE, (u64)fd, 0, 0, 0, 0);
}

int stat(int fd, struct stat *stat) {
    return (int)__syscall(SYS_STAT, (u64)fd, (u64)stat, 0, 0, 0);
}

int fork(void) {
    return (int)__syscall(SYS_FORK, 0, 0, 0, 0, 0);
}

int execve(const char *name, const char **argv, const char **envp) {
    return (int)__syscall(SYS_EXECVE, (u64)name, (u64)argv, (u64)envp, 0, 0);
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
    return (int)__syscall(SYS_GETTIMEOFDAY, (u64)tv, (u64)tz, 0, 0, 0);
}

int settimeofday(const struct timeval *tv, const struct timezone *tz) {
    return (int)__syscall(SYS_SETTIMEOFDAY, (u64)tv, (u64)tz, 0, 0, 0);
}

int mount(const char *src, const char *dst, const char *fstype,
          unsigned long mountflags, const void *data) {
    return (int)__syscall(SYS_MOUNT, (u64)src, (u64)dst, (u64)fstype,
                          (u64)mountflags, (u64)data);
}

int umount(const char *target) {
    return (int)__syscall(SYS_UMOUNT, (u64)target, 0, 0, 0, 0);
}

int mkdir(const char *name, mode_t mode) {
    return (int)__syscall(SYS_MKDIR, (u64)name, (u64)mode, 0, 0, 0);
}

int rmdir(const char *name) {
    return (int)__syscall(SYS_RMDIR, (u64)name, 0, 0, 0, 0);
}

int unlink(const char *name) {
    return (int)__syscall(SYS_UNLINK, (u64)name, 0, 0, 0, 0);
}

int link(const char *oldpath, const char *newpath) {
    return (int)__syscall(SYS_LINK, (u64)oldpath, (u64)newpath, 0, 0, 0);
}

int rename(const char *oldpath, const char *newpath) {
    return (int)__syscall(SYS_RENAME, (u64)oldpath, (u64)newpath, 0, 0, 0);
}

int chmod(const char *filename, mode_t mode) {
    return (int)__syscall(SYS_RENAME, (u64)filename, (u64)mode, 0, 0, 0);
}

int fchmod(int fd, mode_t mode) {
    return (int)__syscall(SYS_FCHMOD, (u64)fd, (u64)mode, 0, 0, 0);
}

off_t lseek(int fd, off_t offset, int whence) {
    return (off_t)__syscall(SYS_LSEEK, (u64)fd, (u64)offset, (u64)whence, 0, 0);
}

mode_t umask(mode_t mask) {
    return (mode_t)__syscall(SYS_LSEEK, (u64)mask, 0, 0, 0, 0);
}

int dup(int oldfd) {
    return (int)__syscall(SYS_LSEEK, (u64)oldfd, 0, 0, 0, 0);
}

int dup2(int oldfd, int newfd) {
    return (int)__syscall(SYS_FCHMOD, (u64)oldfd, (u64)newfd, 0, 0, 0);
}

int fcntl(int fd, int cmd, ...) {
    va_list lst;
    va_start(lst, cmd);
    unsigned long arg = va_arg(lst, unsigned long);
    int result = (int)__syscall(SYS_FCNTL, (u64)fd, (u64)cmd, (u64)arg, 0, 0);
    va_end(lst);
    return result;
}

int ioctl(int fd, int cmd, ...) {
    va_list lst;
    va_start(lst, cmd);
    unsigned long arg = va_arg(lst, unsigned long);
    int result = (int)__syscall(SYS_IOCTL, (u64)fd, (u64)cmd, (u64)arg, 0, 0);
    va_end(lst);
    return result;
}

void *sbrk(intptr_t increment) {
    return (void *)__syscall(SYS_SBRK, (u64)increment, 0, 0, 0, 0);
}
