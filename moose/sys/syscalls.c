#include <moose/arch/cpu.h>
#include <moose/kstdio.h>
#include <moose/sched/sched.h>
#include <moose/sys/syscalls.h>

// Note that altough we do here very evil thing of recasting pointer types,
// we are certain that this would produce the desired behaviour on platforms
// that we want to support (amd64 and aarch64 both), where arguments
// are passed via registers.
static u64 (*syscall_table[])(u64, u64, u64, u64, u64) = {
#define SYSCALL(_name) [__SYS_##_name] = (void *)sys$##_name,
    __ENUMERATE_SYSCALLS
#undef SYSCALL
};

void syscall_handler(struct registers_state *state) {
    struct syscall_parameters params;
    parse_syscall_parameters(state, &params);
    if (params.function > ARRAY_SIZE(syscall_table)) {
        kprintf("Invalid syscall number %lx\n", params.function);
        return;
    }

    u64 result = syscall_table[params.function](
        params.arg0, params.arg1, params.arg2, params.arg3, params.arg4);

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
    struct process *current = get_current();
    spin_lock(&current->lock);
    mode_t prev = current->umask;
    current->umask = mask & 0777;
    spin_unlock(&current->lock);
    return prev;
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

__noreturn void sys$exit(int status) {
    (void)status;
    __builtin_unreachable();
}

int sys$kill(pid_t pid, int sig) {
    (void)pid;
    (void)sig;
    return -1;
}

int sys$wait(int *wstatus) {
    (void)wstatus;
    return -1;
}

ssize_t sys$readlink(const char *pathname, char *buf, size_t bufsiz) {
    (void)pathname;
    (void)buf;
    (void)bufsiz;
    return -1;
}

int sys$fstat(int fd, struct stat *stat) {
    (void)fd;
    (void)stat;
    return -1;
}
