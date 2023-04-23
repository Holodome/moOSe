#include <arch/cpu.h>
#include <sys/syscalls.h>

static u64 handle_syscall(const struct syscall_parameters *params __unused) {
    return 0;
}

void syscall_handler(struct registers_state *state) {
    struct syscall_parameters params;
    parse_syscall_parameters(state, &params);
    u64 result = handle_syscall(&params);
    set_syscall_result(result, state);
}

int sys$open(const char *name, int flags, mode_t mode) {
    (void)name;
    (void)flags;
    (void)mode;
    return -1;
}

int sys$creat(const char *name, mode_t mode) {
    (void)name;
    (void)mode;
    return -1;
}

ssize_t sys$read(int fd, void *buf, size_t count) {
    (void)fd;
    (void)buf;
    (void)(count);
    return -1;
}

ssize_t sys$write(int fd, void *buf, size_t count) {
    (void)fd;
    (void)buf;
    (void)(count);
    return -1;
}

int sys$close(int fd) {
    (void)fd;
    return -1;
}

int sys$stat(int fd, struct stat *stat) {
    (void)fd;
    (void)stat;
    return -1;
}

int sys$fork(void) {
    return -1;
}

int sys$execve(const char *name, const char **argv, const char **envp) {
    (void)name;
    (void)argv;
    (void)envp;
    return -1;
}

int sys$gettimeofday(struct timeval *tv, struct timezone *tz) {
    (void)tv;
    (void)tz;
    return -1;
}

int sys$settimeofday(const struct timeval *tv, const struct timezone *tz) {
    (void)tv;
    (void)tz;
    return -1;
}

int sys$mount(const char *source, const char *dst, const char *fstype,
              unsigned long mountflags, const void *data) {
    (void)source;
    (void)dst;
    (void)fstype;
    (void)mountflags;
    (void)data;
    return -1;
}

int sys$umount(const char *target) {
    (void)target;
    return -1;
}

int sys$mkdir(const char *name, mode_t mode) {
    (void)name;
    (void)mode;
    return -1;
}

int sys$rmdir(const char *name) {
    (void)name;
    return -1;
}

int sys$unlink(const char *name) {
    (void)name;
    return -1;
}

int sys$link(const char *oldpath, const char *newpath) {
    (void)oldpath;
    (void)newpath;
    return -1;
}

int sys$rename(const char *oldpath, const char *newpath) {
    (void)oldpath;
    (void)newpath;
    return -1;
}

int sys$chmod(const char *filename, mode_t mode) {
    (void)filename;
    (void)mode;
    return -1;
}

int sys$fchmod(int fd, mode_t mode) {
    (void)fd;
    (void)mode;
    return -1;
}

off_t sys$lseek(int fd, off_t offset, int whence) {
    (void)fd;
    (void)offset;
    (void)whence;
    return -1;
}

mode_t sys$umask(mode_t mask) {
    (void)mask;
    return -1;
}

int sys$dup(int oldfd) {
    (void)oldfd;
    return -1;
}

int sys$dup2(int oldfd, int newfd) {
    (void)oldfd;
    (void)newfd;
    return -1;
}

int sys$fcntl(int fd, int cmd, unsigned long arg) {
    (void)fd;
    (void)cmd;
    (void)arg;
    return -1;
}

int sys$ioctl(int fd, int cmd, unsigned long arg) {
    (void)fd;
    (void)cmd;
    (void)arg;
    return -1;
}

void *sys$sbrk(intptr_t increment) {
    (void)increment;
    return (void *)(uintptr_t)(-1);
}
