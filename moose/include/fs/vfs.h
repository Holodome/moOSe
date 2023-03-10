#pragma once

#include <list.h>
#include <types.h>
#include <fs/posix.h>

struct sb;
struct inode;
struct file;
struct dentry;

struct sb_ops {
    struct inode (*alloc_inode)(struct sb *sb);
    void (*destroy_inode)(struct inode *inode);
};

struct sb {
    u32 blk_sz;
    u32 blk_sz_bits;

    void *private;
    struct sb_ops *ops;

    struct list_head inode_list;
    struct list_head file_list;
};

struct inode_ops {};

struct inode {
    ino_t num;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    off_t file_size;

    void *private;
    struct inode_ops *ops;
    struct sb *sb;

    struct list_head sb_list;
    struct list_head dentry_list;
};

struct file_ops {
    off_t (*lseek)(struct file *, off_t, int);
    ssize_t (*read)(struct file *, void *, size_t, off_t *);
    ssize_t (*write)(struct file *, const void *, size_t, off_t *);
    int (*open)(struct inode *, struct file *);
    int (*release)(struct inode *, struct file *);
    int (*readdir)(struct file *, struct dentry *);
};

struct file {
    off_t offset;

    void *private;
    struct file_ops *ops;
    struct dentry *dentry;

    struct list_head list;
};

struct dentry {
    struct inode *inode;
    struct dentry *parent;
    const char *name;

    struct list_head dir_list;
    struct list_head subdir_list;
    struct list_head inode_list;
};
