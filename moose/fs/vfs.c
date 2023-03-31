#include "fs/vfs.h"
#include <assert.h>
#include <errno.h>
#include <kstdio.h>
#include <mm/kmalloc.h>

static struct filesystem *filesystem;

int register_filesystem(struct filesystem *fs) {
    filesystem = fs;
    return 0;
}

void fill_kstat(struct inode *inode, struct kstat *stat) {
    stat->st_dev = 0;
    stat->st_ino = inode->ino;
    stat->st_mode = inode->mode;
    stat->st_nlink = inode->nlink;
    stat->st_uid = inode->uid;
    stat->st_gid = inode->gid;
    stat->st_rdev = 0;
    stat->st_size = inode->size;
    stat->st_blksize = inode->sb->blk_sz;
    stat->st_blkcnt = inode->block_count;
    stat->st_atim = inode->atime;
    stat->st_mtim = inode->mtime;
    stat->st_ctim = inode->ctime;
}

off_t generic_lseek(struct file *filp, off_t offset, int whence) {
    off_t new_offset;
    switch (whence) {
    case SEEK_SET:
        if (offset < 0)
            offset = 0;
        new_offset = offset;
        break;
    case SEEK_CUR:
        if (offset < 0 && filp->offset < -offset)
            new_offset = 0;
        else
            new_offset = filp->offset + offset;
        break;
    case SEEK_END:
        new_offset = filp->dentry->inode->size;
        break;
    default:
        return -EINVAL;
    }

    assert(new_offset >= 0);
    filp->offset = new_offset;
    return 0;
}

struct file *get_empty_filp(void) {
    struct file *filp = kzalloc(sizeof(*filp));
    if (!filp)
        return NULL;

    init_list_head(&filp->sb_list);
    return filp;
}

struct dentry *create_dentry(struct dentry *parent, const char *str) {
    expects(parent);
    expects(str);
    struct dentry *entry = kzalloc(sizeof(*entry));
    if (!entry)
        return NULL;
    entry->name = kstrdup(str);
    if (!entry->name)
        goto err_dentry;

    entry->parent = parent;

    return entry;
err_dentry:
    kfree(entry);
    return NULL;
}

struct dentry *create_root_dentry(void) {
    struct dentry *entry = kzalloc(sizeof(*entry));
    if (!entry)
        return NULL;

    return entry;
}

struct superblock *vfs_mount(struct blk_device *dev,
                             int (*mount)(struct superblock *)) {
    struct superblock *sb = kzalloc(sizeof(*sb));
    if (!sb)
        return ERR_PTR(-ENOMEM);

    sb->dev = dev;
    init_list_head(&sb->inode_list);
    init_list_head(&sb->file_list);
    refcount_set(&sb->refcnt, 1);

    int result = mount(sb);
    if (result < 0) {
        kfree(sb);
        return ERR_PTR(result);
    }

    return sb;
}

void vfs_umount(struct superblock *sb) {
    expects(refcount_read(&sb->refcnt) == 0);
    sb->ops->release_sb(sb);
}

struct inode *alloc_inode(void) {
    struct inode *inode = kzalloc(sizeof(*inode));
    if (!inode)
        return NULL;

    refcount_set(&inode->refcnt, 1);
    init_list_head(&inode->sb_list);

    return inode;
}

void release_inode(struct inode *inode) {
    if (refcount_dec_and_test(&inode->refcnt)) {
        list_remove(&inode->sb_list);
        inode->ops->free(inode);
        release_sb(inode->sb);
        kfree(inode);
    }
}

void release_sb(struct superblock *sb) {
    if (refcount_dec_and_test(&sb->refcnt)) {
        sb->ops->release_sb(sb);
        kfree(sb);
    }
}

void release_dentry(struct dentry *entry) {
    if (refcount_dec_and_test(&entry->refcnt)) {
        release_inode(entry->inode);
        if (entry->parent)
            release_dentry(entry->parent);
        kfree(entry->name);
        kfree(entry);
    }
}

struct file *vfs_open_dentry(struct dentry *entry) {
    struct file *filp = get_empty_filp();
    if (!filp)
        return ERR_PTR(-ENOMEM);

    refcount_inc(&entry->refcnt);
    filp->dentry = entry;
    filp->ops = entry->inode->file_ops;

    return filp;
}

void init_dentry(struct dentry *entry, struct inode *inode) {
    refcount_inc(&inode->refcnt);
    entry->inode = inode;
}

int dentry_set_name(struct dentry *entry, const char *name) {
    char *new_name = kstrdup(name);
    if (!new_name)
        return -1;
    entry->name = new_name;
    return 0;
}

struct dentry *vfs_readdir(struct file *filp) {
    struct dentry *entry = kmalloc(sizeof(*entry));
    if (!entry)
        return ERR_PTR(-ENOMEM);
    int err = filp->ops->readdir(filp, entry);
    if (err) {
        kfree(entry);
        return ERR_PTR(err);
    }

    return entry;
}

void print_inode(const struct inode *inode) {
    kprintf("inode ino=%lu\n", (unsigned long)inode->ino);
    kprintf(" mode=%lu uid=%lu gid=%lu\n", (unsigned long)inode->mode,
            (unsigned long)inode->uid, (unsigned long)inode->gid);
    kprintf(" size=%ld nlink=%ld blkcnt=%ld\n", (unsigned long)inode->size,
            (unsigned long)inode->nlink, (unsigned long)inode->block_count);
}
