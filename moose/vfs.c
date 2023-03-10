#include "fs/vfs.h"

#include <assert.h>

off_t lseek(struct file *filp, off_t offset, int whence) {
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
