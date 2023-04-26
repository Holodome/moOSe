#pragma once

#include <moose/arch/refcount.h>
#include <moose/fs/posix.h>
#include <moose/list.h>
#include <moose/types.h>

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
    // Apply changed 'size'
    int (*truncate)(struct inode *inode);
    // Find child in directory
    int (*lookup)(struct inode *dir, struct dentry *entry);
    // Apply changed inode attributes
    int (*setattr)(struct inode *inode);
    int (*mkdir)(struct inode *dir, struct dentry *entry, mode_t mode);
    int (*rmdir)(struct inode *dir, struct dentry *entry);
    int (*create)(struct inode *dir, struct dentry *entry, mode_t mode);
    int (*mknod)(struct inode *dir, struct dentry *entry, mode_t mode,
                 dev_t dev);
    int (*rename)(struct inode *olddir, struct dentry *oldentry,
                  struct inode *newdir, struct dentry *newentry);
    int (*unlink)(struct inode *dir, struct dentry *entry);
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
    off_t (*lseek)(struct file *filp, off_t offset, int whence);
    ssize_t (*read)(struct file *filp, void *buf, size_t count);
    ssize_t (*write)(struct file *filp, const void *buf, size_t count);
    void (*release)(struct file *filp);
    int (*readdir)(struct file *dir, struct dentry *entry);
    int (*open)(struct inode *inode, struct file *filp);
};

struct file {
    refcount_t refcnt;
    off_t offset;

    void *private;
    const struct file_ops *ops;
    struct dentry *dentry;

    struct list_head sb_list;
};

struct dentry {
    refcount_t refcnt;

    struct dentry *parent;
    struct inode *inode;
    char *name;

    struct list_head inode_list;
};

static __forceinline struct inode *file_inode(const struct file *filp) {
    return filp->dentry->inode;
}

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
void init_dentry(struct dentry *entry, struct inode *inode);
int dentry_set_name(struct dentry *entry, const char *name);
void release_sb(struct superblock *sb);
void release_inode(struct inode *inode);
void release_dentry(struct dentry *entry);

void print_inode(const struct inode *inode);

static __forceinline off_t __block_end(struct superblock *sb, off_t cursor) {
    cursor += (sb->blk_sz - cursor % sb->blk_sz);
    return cursor;
}
