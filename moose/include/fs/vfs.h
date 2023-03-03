#pragma once

#include <types.h>

struct vfs_sb;
struct vfs_inode;
struct vfs_file;
struct vfs_dentry;

struct vfs_sb_ops {
    struct vfs_inode (*alloc_inode)(struct vfs_sb *sb);
    void (*destroy_inode)(struct vfs_inode *inode);
};

struct vfs_sb {
    u32 blk_sz;
    u32 blk_sz_bits;

    struct vfs_sb_ops *ops;

    struct list_head inode_list;
    struct list_head file_list;
};

struct vfs_inode_ops {
};

struct vfs_inode {
    u64 num;
};

struct vfs_file_ops {

};
