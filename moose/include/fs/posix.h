#pragma once

#include <types.h>

// <errno.h>

#define ERRLIST                                                                \
    E(EILSEQ, "Illegal byte sequence")                                         \
    E(EDOM, "Domain error")                                                    \
    E(ERANGE, "Result not representable")                                      \
    E(ENOTTY, "Not a tty")                                                     \
    E(EACCES, "Permission denied")                                             \
    E(EPERM, "Operation not permitted")                                        \
    E(ENOENT, "No such file or directory")                                     \
    E(ESRCH, "No such process")                                                \
    E(EEXIST, "File exists")                                                   \
    E(EOVERFLOW, "Value too large for data type")                              \
    E(ENOSPC, "No space left on device")                                       \
    E(ENOMEM, "Out of memory")                                                 \
    E(EBUSY, "Resource busy")                                                  \
    E(EINTR, "Interrupted system call")                                        \
    E(EAGAIN, "Resource temporarily unavailable")                              \
    E(ESPIPE, "Invalid seek")                                                  \
    E(EXDEV, "Cross-device link")                                              \
    E(EROFS, "Read-only file system")                                          \
    E(ENOTEMPTY, "Directory not empty")                                        \
    E(ECONNRESET, "Connection reset by peer")                                  \
    E(ETIMEDOUT, "Operation timed out")                                        \
    E(ECONNREFUSED, "Connection refused")                                      \
    E(EHOSTDOWN, "Host is down")                                               \
    E(EHOSTUNREACH, "Host is unreachable")                                     \
    E(EADDRINUSE, "Address in use")                                            \
    E(EPIPE, "Broken pipe")                                                    \
    E(EIO, "I/O error")                                                        \
    E(ENXIO, "No such device or address")                                      \
    E(ENOTBLK, "Block device required")                                        \
    E(ENODEV, "No such device")                                                \
    E(ENOTDIR, "Not a directory")                                              \
    E(EISDIR, "Is a directory")                                                \
    E(ETXTBSY, "Text file busy")                                               \
    E(ENOEXEC, "Exec format error")                                            \
    E(EINVAL, "Invalid argument")                                              \
    E(E2BIG, "Argument list too long")                                         \
    E(ELOOP, "Symbolic link loop")                                             \
    E(ENAMETOOLONG, "Filename too long")                                       \
    E(ENFILE, "Too many open files in system")                                 \
    E(EMFILE, "No file descriptors available")                                 \
    E(EBADF, "Bad file descriptor")                                            \
    E(ECHILD, "No child process")                                              \
    E(EFAULT, "Bad address")                                                   \
    E(EFBIG, "File too large")                                                 \
    E(EMLINK, "Too many links")                                                \
    E(ENOLCK, "No locks available")                                            \
    E(EDEADLK, "Resource deadlock would occur")                                \
    E(ENOTRECOVERABLE, "State not recoverable")                                \
    E(EOWNERDEAD, "Previous owner died")                                       \
    E(ECANCELED, "Operation canceled")                                         \
    E(ENOSYS, "Function not implemented")                                      \
    E(ENOMSG, "No message of desired type")                                    \
    E(EIDRM, "Identifier removed")                                             \
    E(ENOSTR, "Device not a stream")                                           \
    E(ENODATA, "No data available")                                            \
    E(ETIME, "Device timeout")                                                 \
    E(ENOSR, "Out of streams resources")                                       \
    E(ENOLINK, "Link has been severed")                                        \
    E(EPROTO, "Protocol error")                                                \
    E(EBADMSG, "Bad message")                                                  \
    E(EBADFD, "File descriptor in bad state")                                  \
    E(ENOTSOCK, "Not a socket")                                                \
    E(EDESTADDRREQ, "Destination address required")                            \
    E(EMSGSIZE, "Message too large")                                           \
    E(EPROTOTYPE, "Protocol wrong type for socket")                            \
    E(ENOPROTOOPT, "Protocol not available")                                   \
    E(EPROTONOSUPPORT, "Protocol not supported")                               \
    E(ESOCKTNOSUPPORT, "Socket type not supported")                            \
    E(ENOTSUP, "Not supported")                                                \
    E(EPFNOSUPPORT, "Protocol family not supported")                           \
    E(EAFNOSUPPORT, "Address family not supported by protocol")                \
    E(EADDRNOTAVAIL, "Address not available")                                  \
    E(ENETDOWN, "Network is down")                                             \
    E(ENETUNREACH, "Network unreachable")                                      \
    E(ENETRESET, "Connection reset by network")                                \
    E(ECONNABORTED, "Connection aborted")                                      \
    E(ENOBUFS, "No buffer space available")                                    \
    E(EISCONN, "Socket is connected")                                          \
    E(ENOTCONN, "Socket not connected")                                        \
    E(ESHUTDOWN, "Cannot send after socket shutdown")                          \
    E(EALREADY, "Operation already in progress")                               \
    E(EINPROGRESS, "Operation in progress")                                    \
    E(ESTALE, "Stale file handle")                                             \
    E(EREMOTEIO, "Remote I/O error")                                           \
    E(EDQUOT, "Quota exceeded")                                                \
    E(ENOMEDIUM, "No medium found")                                            \
    E(EMEDIUMTYPE, "Wrong medium type")                                        \
    E(EMULTIHOP, "Multihop attempted")                                         \
    E(ENOKEY, "Required key not available")                                    \
    E(EKEYEXPIRED, "Key has expired")                                          \
    E(EKEYREVOKED, "Key has been revoked")                                     \
    E(EKEYREJECTED, "Key was rejected by service")

enum {
    __ERRNOOK = 0,
#define E(_name, ...) _name,
    ERRLIST
#undef E
};

#define MAX_ERRNO 4095

// non-posix linux pointer error handling

#define IS_PTR_ERR(_ptr)                                                       \
    __unlikely((uintptr_t)(void *)(_ptr) >= (uintptr_t)(-MAX_ERRNO))
#define IS_PTR_ERR_OR_NULL(_ptr) (__unlikely(!(_ptr)) || IS_PTR_ERR(_ptr))
static __always_inline __nodiscard void *ERR_PTR(int err) {
    return (void *)(uintptr_t)err;
}
static __always_inline __nodiscard int PTR_ERR(const void *ptr) {
    return (int)(uintptr_t)ptr;
}

// <stdio.h>

#define SEEK_SET 0
#define SEEK_END 1
#define SEEK_CUR 2

// <sys/types.h>

typedef u32 uid_t;
typedef u32 gid_t;
typedef u64 ino_t;
typedef i64 off_t;

typedef u32 blkcnt_t;
typedef u32 blcksize_t;
typedef u32 dev_t;
typedef u16 mode_t;
typedef u32 nlink_t;

typedef i64 time_t;
typedef u32 useconds_t;
typedef i32 iseconds_t;
typedef u32 clock_t;

typedef i64 off_t;

// <time.h>

struct ktm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int im_isdt;
};

struct ktimespec {
    time_t tv_sec;
    long tv_nsec;
};

// <sys/stat.h>

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
    blcksize_t st_blksize;
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

