#pragma once

#include <arch/refcount.h>
#include <fs/posix.h>
#include <list.h>
#include <types.h>

struct superblock;
struct inode;
struct file;
struct dentry;
struct blk_device;

struct sb_ops {
    void (*release_sb)(struct superblock *sb);
};

struct superblock {
    refcount_t refcnt;

    u32 blk_sz;
    u32 blk_sz_bits;

    struct dentry *root;

    void *private;
    const struct sb_ops *ops;
    struct blk_device *dev;

    struct list_head inode_list;
    struct list_head file_list;
};

struct inode_ops {
    void (*free)(struct inode *inode);
    int (*lookup)(struct inode *inode, struct dentry *dentry);
};

struct inode {
    refcount_t refcnt;

    ino_t ino;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    off_t size;
    nlink_t nlink;
    blkcnt_t block_count;
    struct ktimespec atime;
    struct ktimespec mtime;
    struct ktimespec ctime;

    void *private;
    const struct inode_ops *ops;
    const struct file_ops *file_ops;
    struct superblock *sb;

    struct list_head sb_list;
    struct list_head dentry_list;
};

struct file_ops {
    off_t (*lseek)(struct file *, off_t, int);
    ssize_t (*read)(struct file *, void *, size_t);
    ssize_t (*write)(struct file *, const void *, size_t);
    int (*release)(struct inode *, struct file *);
    struct dentry *(*readdir)(struct file *);
};

struct file {
    refcount_t refcnt;
    off_t offset;

    void *private;
    const struct file_ops *ops;
    struct dentry *dentry;

    struct list_head list;
};

struct dentry {
    refcount_t refcnt;

    struct inode *inode;
    struct dentry *parent;
    char *name;

    struct list_head dir_list;
    struct list_head subdir_list;
    struct list_head inode_list;
};

struct superblock *vfs_mount(struct blk_device *dev,
                             int (*mount)(struct superblock *));
void vfs_umount(struct superblock *sb);
int vfs_create(struct inode *dir, struct dentry *entry, mode_t mode);
int vfs_mkdir(struct inode *dir, struct dentry *entry, mode_t mode);
int vfs_rmdir(struct inode *dir, struct dentry *entry);
struct dentry *vfs_readdir(struct file *filp);
struct file *vfs_open_dentry(struct dentry *entry);

struct file *get_empty_filp(void);
void fill_kstat(struct inode *inode, struct kstat *stat);
off_t generic_lseek(struct file *filp, off_t offset, int whence);

struct dentry *create_dentry(struct dentry *parent, const char *str);
struct dentry *create_root_dentry(void);
struct inode *alloc_inode(void);
void release_sb(struct superblock *sb);
void release_inode(struct inode *inode);
void release_dentry(struct dentry *entry);

void print_inode(const struct inode *inode);
