#include <fs/ramfs.h>

#include <mm/kmalloc.h>

#define RAMFS_BLOCK_SIZE 4096
#define RAMFS_BLOCK_SIZE_BITS 12

static const struct sb_ops sb_ops = {.release_sb = ext2_release_sb};
static const struct inode_ops inode_ops = {.free = ext2_free_inode};
static const struct file_ops file_ops = {.lseek = generic_lseek,
                                         .read = ext2_read,
                                         .write = ext2_write,
                                         .readdir = ext2_readdir};

struct ramfs_block {
    struct list_head list;
    size_t idx;
    char data[RAMFS_BLOCK_SIZE];
};

struct ramfs_inode {
    size_t block_count;
    struct list_head block_list;
};

struct ramfs {
    ino_t inode_counter;
    struct list_head block_freelist;
};

static struct inode *ramfs_create_inode(void) {
    struct inode *inode = alloc_inode();
    if (!inode) return NULL;

    struct ramfs_inode *ri = kzalloc(sizeof(*ri));
    if (!ri) {
        kfree(inode);
        return NULL;
    }
    init_list_head(&ri->block_list);
    inode->private = ri;
    return inode;
}

static struct dentry *ramfs_create_root(void) {
    struct dentry *entry = create_root_dentry();
    if (!entry) return NULL;

    struct inode *inode = ramfs_create_inode();
    if (!inode) {
        kfree(entry);
        return NULL;
    }

    init_dentry(entry, inode);
    return entry;
}

int ramfs_mount(struct superblock *sb) {
    struct ramfs *fs = kzalloc(sizeof(*fs));
    if (!fs) return -ENOMEM;

    init_list_head(&fs->block_freelist);
    sb->private = fs;
    sb->blk_sz = RAMFS_BLOCK_SIZE;
    sb->blk_sz_bits = RAMFS_BLOCK_SIZE_BITS;
    sb->ops = &sb_ops;

    struct dentry *root = ramfs_create_root();
    if (!root) {
        kfree(fs);
        return -ENOMEM;
    }

    return 0;
}
