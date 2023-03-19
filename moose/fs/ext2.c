#include <fs/ext2.h>

#include <assert.h>
#include <bitops.h>
#include <blk_device.h>
#include <endian.h>
#include <fs/vfs_impl.h>
#include <kstdio.h>
#include <mm/kmalloc.h>

#define EXT2_DIRECT_BLOCKS 12
#define EXT2_SB_OFFSET 1024
#define EXT2_BGD_OFFSET 2048

// The first few entries of the inode tables are reserved. In revision 0 there
// are 11 entries reserved while in revision 1 (EXT2_DYNAMIC_REV) and later the
// number of reserved inodes entries is specified in the s_first_ino of the
// superblock structure
#define EXT2_FIRST_INO 11

static ssize_t ext2_write(struct file *filp, const void *buf, size_t count);
static ssize_t ext2_read(struct file *filp, void *buf, size_t count);
static void ext2_release_sb(struct superblock *sb);
static struct dentry *ext2_inode_lookup(struct inode *inode,
                                        struct dentry *dentry);
static void ext2_free_inode(struct inode *inode);
static const struct sb_ops sb_ops = {.release_sb = ext2_release_sb};
static const struct inode_ops inode_ops = {.free = ext2_free_inode,
                                           .lookup = ext2_inode_lookup};
static const struct file_ops file_ops = {
    .lseek = generic_lseek, .read = ext2_read, .write = ext2_write};

static void sync_superblock(struct superblock *);

#define ext2_error(_sb, _fmt, ...)                                             \
    ext2_error_(_sb, __PRETTY_FUNCTION__, _fmt, ##__VA_ARGS__)
static void ext2_error_(struct superblock *sb, const char *func_name,
                        const char *fmt, ...) {
    struct ext2_fs *ext2 = sb->private;
    ext2->sb.s_state |= EXT2_ERROR_FS;
    sync_superblock(sb);

    va_list args;
    va_start(args, fmt);
    kprintf("ext2 error in %s: ", func_name);
    kvprintf(fmt, args);
    kputc('\n');
    va_end(args);
}

static struct ktimespec ext2_to_timespect(u32 raw) {
    struct ktimespec timespec;
    timespec.tv_sec = raw;
    timespec.tv_nsec = 0;
    return timespec;
}

static void calc_block_group(const struct ext2_fs *fs, blkcnt_t block,
                             u32 *group, u32 *in_group) {
    *group = block / fs->sb.s_blocks_per_group;
    *in_group = block % fs->sb.s_blocks_per_group;
}

static void calc_ino_group(const struct ext2_fs *fs, ino_t ino, u32 *group,
                           u32 *in_group) {
    expects(ino);
    --ino;
    *group = ino / fs->sb.s_inodes_per_group;
    *in_group = ino % fs->sb.s_inodes_per_group;
}

static void read_superblock(struct superblock *fs, struct ext2_sb *sb) {
    blk_read(fs->dev, EXT2_SB_OFFSET, sb, sizeof(*sb));
}

static void sync_superblock(struct superblock *fs) {
    struct ext2_fs *ext2 = fs->private;
    blk_write(fs->dev, EXT2_SB_OFFSET, &ext2->sb, sizeof(ext2->sb));
}

// note: need to provide is_dir because ext2 block group tracks directory count
__attribute__((used)) static ssize_t alloc_ino(struct superblock *sb,
                                               int is_dir) {
    struct ext2_fs *ext2 = sb->private;

    size_t bgdi;
    struct ext2_group_desc *desc = NULL;
    for (bgdi = 0; bgdi < ext2->bgds_count && !desc; ++bgdi) {
        struct ext2_group_desc *test = ext2->bgds + bgdi;
        if (!test->bg_free_inodes_count) continue;

        desc = test;
    }

    if (desc == NULL) return -ENOSPC;

    u64 bitmap[ext2->group_inode_bitmap_size];
    blk_read(sb->dev, desc->bg_inode_bitmap << sb->blk_sz_bits, bitmap,
             sizeof(bitmap));

    u64 found = bitmap_first_clear(bitmap, ext2->sb.s_inodes_per_group);
    if (!found) ext2_error(sb, "corrupted inode bitmap");

    if (desc->bg_free_inodes_count)
        --desc->bg_free_inodes_count;
    else
        ext2_error(sb, "currupted block group free inode count");

    if (ext2->sb.s_free_inode_count)
        --ext2->sb.s_free_inode_count;
    else
        ext2_error(sb, "corrupted superblock free inode count");

    if (is_dir) --desc->bg_used_dirs_count;
    set_bit(found, bitmap);

    blk_write(sb->dev, desc->bg_inode_bitmap << sb->blk_sz_bits, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);

    return bgdi * ext2->sb.s_inodes_per_group + found;
}

__attribute__((used)) static void free_ino(struct superblock *sb, ino_t ino,
                                           int is_dir) {
    struct ext2_fs *ext2 = sb->private;
    u32 ino_group, ino_in_group;
    calc_ino_group(ext2, ino, &ino_group, &ino_in_group);
    expects(ino_group < ext2->bgds_count);

    struct ext2_group_desc *group = ext2->bgds + ino_group;
    u64 bitmap[ext2->group_inode_bitmap_size];
    blk_read(sb->dev, group->bg_inode_bitmap << sb->blk_sz_bits, bitmap,
             sizeof(bitmap));

    if (!test_bit(ino_in_group, bitmap))
        ext2_error(sb, "corrupted inode bitmap");

    clear_bit(ino_in_group, bitmap);
    ++group->bg_free_inodes_count;
    if (is_dir && !group->bg_used_dirs_count)
        ext2_error(sb, "corrupted block group used dir count");

    group->bg_used_dirs_count -= is_dir;
    ++ext2->sb.s_free_inode_count;

    blk_write(sb->dev, group->bg_inode_bitmap << sb->blk_sz_bits, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);
}

__attribute__((used)) static ssize_t alloc_block(struct superblock *sb) {
    struct ext2_fs *ext2 = sb->private;
    size_t bgdi;
    struct ext2_group_desc *desc = NULL;
    for (bgdi = 0; bgdi < ext2->bgds_count && !desc; ++bgdi) {
        struct ext2_group_desc *test = ext2->bgds + bgdi;
        if (!test->bg_free_blocks_count) continue;

        desc = test;
    }

    if (desc == NULL) return -ENOSPC;

    u64 bitmap[ext2->group_block_bitmap_size];
    blk_read(sb->dev, desc->bg_block_bitmap << sb->blk_sz_bits, bitmap,
             sizeof(bitmap));

    u64 found = bitmap_first_clear(bitmap, ext2->sb.s_blocks_per_group);
    if (!found) ext2_error(sb, "corrupted block bitmap");

    if (desc->bg_free_blocks_count)
        --desc->bg_free_blocks_count;
    else
        ext2_error(sb, "currupted block group free block count");

    if (ext2->sb.s_free_block_count)
        --ext2->sb.s_free_block_count;
    else
        ext2_error(sb, "corrupted superblock free block count");

    set_bit(found, bitmap);

    blk_write(sb->dev, desc->bg_block_bitmap << sb->blk_sz_bits, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);

    return bgdi * ext2->sb.s_blocks_per_group + found - 1;
}

__attribute__((used)) static void free_block(struct superblock *sb,
                                             blkcnt_t block) {
    struct ext2_fs *fs = sb->private;
    u32 block_group, block_in_group;
    calc_block_group(fs, block, &block_group, &block_in_group);

    expects(block_group < fs->bgds_count);
    struct ext2_group_desc *group = fs->bgds + block_group;
    u64 bitmap[fs->group_block_bitmap_size];
    blk_read(sb->dev, group->bg_block_bitmap << sb->blk_sz_bits, bitmap,
             sizeof(bitmap));

    if (!test_bit(block_in_group, bitmap))
        ext2_error(sb, "corrupted block bitmap");

    clear_bit(block_in_group, bitmap);
    ++group->bg_free_blocks_count;
    ++fs->sb.s_free_block_count;

    blk_write(sb->dev, group->bg_block_bitmap << sb->blk_sz_bits, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);
}

static struct ext2_inode *ext2_get_raw_inode(struct superblock *sb, ino_t ino) {
    struct ext2_fs *ext2 = sb->private;
    if (ino == EXT2_ROOT_INO || ino < EXT2_FIRST_INO) {
        ext2_error(sb, "invalid ino %lu\n", (unsigned long)ino);
        return ERR_PTR(-EINVAL);
    }

    u32 ino_group, ino_in_group;
    calc_ino_group(ext2, ino, &ino_group, &ino_in_group);
    expects(ino_group < ext2->bgds_count);
    struct ext2_group_desc *bgd = ext2->bgds + ino_group;

    off_t inode_offset = (bgd->bg_inode_table << sb->blk_sz_bits) +
                         ino_in_group * sizeof(struct ext2_inode);
    struct ext2_inode *inode = kmalloc(sizeof(*inode));
    if (!inode) {
        ext2_error(sb, "inode ino=%lu allocation failed\n", (unsigned long)ino);
        return ERR_PTR(-ENOMEM);
    }

    blk_read(sb->dev, inode_offset, inode, sizeof(*inode));
    return inode;
}

static struct inode *ext2_iget(struct superblock *sb, ino_t ino) {
    struct ext2_inode *ei = ext2_get_raw_inode(sb, ino);
    if (IS_PTR_ERR(ei)) return ERR_PTR_RECAST(ei);

    struct inode *inode = alloc_inode();
    if (!inode) {
        kfree(ei);
        return ERR_PTR(-ENOMEM);
    }

    inode->ino = ino;
    inode->mode = ei->i_mode;
    inode->uid = ei->i_uid;
    inode->gid = ei->i_gid;
    inode->size = ei->i_size;
    inode->nlink = ei->i_links_count;
    inode->block_count = ei->i_blocks;
    inode->atime = ext2_to_timespect(ei->i_atime);
    inode->mtime = ext2_to_timespect(ei->i_mtime);
    inode->ctime = ext2_to_timespect(ei->i_ctime);

    inode->private = ei;
    inode->ops = &inode_ops;
    inode->file_ops = &file_ops;
    inode->sb = sb;

    return inode;
}

static int ext2_do_mount(struct superblock *sb) {
    struct ext2_fs *ext2 = sb->private;
    read_superblock(sb, &ext2->sb);

    size_t bgds_count = sb->dev->capacity / (8 << ext2->sb.s_log_block_size);
    expects(bgds_count != 0);
    size_t bgds_size = bgds_count * sizeof(struct ext2_group_desc);
    struct ext2_group_desc *bgds = kmalloc(bgds_size);
    if (!bgds) return -ENOMEM;

    blk_read(sb->dev, EXT2_BGD_OFFSET, bgds, bgds_size);

    ext2->bgds_count = bgds_count;
    ext2->bgds = bgds;

    u32 inodes_per_group = ext2->sb.s_inodes_per_group;
    u32 blk_sz = 1 << ext2->sb.s_log_block_size;
    ext2->inodes_per_block = blk_sz / sizeof(struct ext2_inode);
    ext2->group_inode_bitmap_size = BITS_TO_BITMAP(inodes_per_group);
    ext2->group_inode_bitmap_size = BITS_TO_BITMAP(inodes_per_group);
    ext2->blocks_per_inderect_block = blk_sz / sizeof(u32);
    ext2->first_2lev_inderect_block = 12 + ext2->blocks_per_inderect_block;
    ext2->first_3lev_inderect_block =
        ext2->first_2lev_inderect_block +
        ext2->blocks_per_inderect_block * ext2->blocks_per_inderect_block;

    return 0;
}

// TODO: Report unallocated blocks (out of bounds)
// TODO: Support implicitly allocated block (filled with 0s)
static blkcnt_t ext2_get_disk_blk(struct ext2_inode *inode,
                                  struct superblock *sb, blkcnt_t blk) {
    struct ext2_fs *ext2 = sb->private;
    blkcnt_t result;

    if (blk < EXT2_DIRECT_BLOCKS) {
        result = inode->i_block[blk];
    } else if (blk < ext2->first_2lev_inderect_block) {
        off_t table_offset = inode->i_block[12] * sb->blk_sz;
        blkcnt_t l1_idx = blk - EXT2_DIRECT_BLOCKS;
        u32 block;
        blk_read(sb->dev, table_offset + l1_idx * sizeof(u32), &block,
                 sizeof(block));

        result = block;
    } else if (blk < ext2->first_3lev_inderect_block) {
        off_t l1_table_offset = inode->i_block[13] * sb->blk_sz;
        blk -= ext2->first_2lev_inderect_block;
        blkcnt_t l1_idx = blk / ext2->blocks_per_inderect_block;
        blkcnt_t l2_idx = blk % ext2->blocks_per_inderect_block;

        u32 l1;
        blk_read(sb->dev, l1_table_offset + l1_idx * sizeof(u32), &l1,
                 sizeof(l1));
        off_t l2_table_offset = l1 * sb->blk_sz;
        u32 l2;
        blk_read(sb->dev, l2_table_offset + l2_idx * sizeof(u32), &l2,
                 sizeof(l2));

        result = l2;
    } else {
        off_t l1_table_offset = inode->i_block[14] * sb->blk_sz;
        blk -= ext2->first_3lev_inderect_block;
        blkcnt_t l1_idx = blk / (ext2->blocks_per_inderect_block *
                                 ext2->blocks_per_inderect_block);
        blk %=
            ext2->blocks_per_inderect_block * ext2->blocks_per_inderect_block;
        blkcnt_t l2_idx = blk / ext2->blocks_per_inderect_block;
        blkcnt_t l3_idx = blk % ext2->blocks_per_inderect_block;

        u32 l1;
        blk_read(sb->dev, l1_table_offset + l1_idx * sizeof(u32), &l1,
                 sizeof(l1));
        off_t l2_table_offset = l1 << sb->blk_sz_bits;
        u32 l2;
        blk_read(sb->dev, l2_table_offset + l2_idx * sizeof(u32), &l2,
                 sizeof(l2));
        off_t l3_table_offset = l2 << sb->blk_sz_bits;
        u32 l3;
        blk_read(sb->dev, l3_table_offset + l3_idx * sizeof(u32), &l3,
                 sizeof(l3));

        result = l3;
    }

    return result;
}

static size_t ext2_read_in_block(struct ext2_inode *inode,
                                 struct superblock *sb, off_t cursor, void *buf,
                                 size_t count) {
    off_t cursor_in_block = cursor % sb->blk_sz;
    off_t current_block = cursor / sb->blk_sz;
    size_t to_read = __block_end(sb, cursor) - cursor;
    if (to_read > count) to_read = count - cursor;

    off_t phys_offset =
        ext2_get_disk_blk(inode, sb, current_block) * sb->blk_sz;
    blk_read(sb->dev, phys_offset + cursor_in_block, buf, to_read);
    return to_read;
}

static size_t ext2_write_in_block(struct ext2_inode *inode,
                                  struct superblock *sb, off_t cursor,
                                  const void *buf, size_t count) {
    off_t cursor_in_block = cursor % sb->blk_sz;
    off_t current_block = cursor / sb->blk_sz;
    size_t to_write = __block_end(sb, cursor) - cursor;
    if (to_write > count) to_write = count - cursor;

    off_t phys_offset =
        ext2_get_disk_blk(inode, sb, current_block) * sb->blk_sz;
    blk_write(sb->dev, phys_offset + cursor_in_block, buf, to_write);
    return to_write;
}

ssize_t ext2_read(struct file *filp, void *buf, size_t count) {
    struct inode *inode = filp->dentry->inode;
    struct superblock *sb = inode->sb;
    struct ext2_inode *ei = inode->private;

    // TODO: Check file size
    char *cursor = buf;
    while (count) {
        size_t read = ext2_read_in_block(ei, sb, filp->offset, cursor, count);
        cursor += read;
        filp->offset += read;
        count -= read;
    }

    return cursor - (char *)buf;
}

ssize_t ext2_write(struct file *filp, const void *buf, size_t count) {
    struct inode *inode = filp->dentry->inode;
    struct superblock *sb = inode->sb;
    struct ext2_inode *ei = inode->private;

    // TODO: Check file size
    const char *cursor = buf;
    while (count) {
        size_t wrote = ext2_write_in_block(ei, sb, filp->offset, cursor, count);
        cursor += wrote;
        filp->offset += wrote;
        count -= wrote;
    }

    return cursor - (char *)buf;
}

int ext2_open(struct inode *, struct file *);
int ext2_release(struct inode *, struct file *);
int ext2_readdir(struct file *, struct dentry *);

int ext2_mount(struct superblock *sb) {
    expects(sb->dev);
    struct ext2_fs *ext2 = kzalloc(sizeof(*ext2));
    if (!ext2) {
        kfree(sb);
        return -ENOMEM;
    }
    sb->private = ext2;
    int result = ext2_do_mount(sb);
    if (result) {
        kfree(sb);
        kfree(ext2);
        return result;
    }

    sb->ops = sb_ops;
    sb->blk_sz_bits = ext2->sb.s_log_block_size;
    sb->blk_sz = 1 << sb->blk_sz_bits;

    return 0;
}

static void ext2_release_sb(struct superblock *sb) {
    struct ext2_fs *ext2 = sb->private;
    kfree(ext2->bgds);
}

