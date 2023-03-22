#pragma once

#include <bitops.h>
#include <fs/vfs.h>

static inline off_t __block_end(struct superblock *sb, off_t cursor) {
    cursor += (sb->blk_sz - cursor % sb->blk_sz);
    return cursor;
}
