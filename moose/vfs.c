#include "fs/vfs.h"

#include <assert.h>

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

off_t __generic_lseek(struct file *filp, off_t offset, int whence) {
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
        new_offset = filp->dentry->inode->file_size;
        break;
    default:
        return -EINVAL;
    }

    assert(new_offset >= 0);
    filp->offset = new_offset;
    return 0;
}

