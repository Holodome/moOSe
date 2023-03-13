#include "fs/vfs.h"

#include <assert.h>
#include <mm/kmalloc.h>

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
        if (offset < 0) offset = 0;
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
    if (!filp) return NULL;

    init_list_head(&filp->list);
    return filp;
}

struct dentry *create_dentry(struct dentry *parent, const char *str) {
    expects(parent != NULL);

    struct dentry *entry = kzalloc(sizeof(*entry));
    if (!entry) return NULL;
    entry->name = strdup(str);
    if (!entry->name) goto err_dentry;

    init_list_head(&entry->dir_list);
    init_list_head(&entry->subdir_list);
    init_list_head(&entry->inode_list);
    entry->parent = parent;
    list_add(&entry->subdir_list, &parent->subdir_list);

    return entry;
err_dentry:
    kfree(entry);
    return NULL;
}

void init_dentry(struct dentry *entry, struct inode *inode) {
    entry->inode = inode;
}
