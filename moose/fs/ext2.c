#include <fs/ext2.h>

#include <assert.h>
#include <bitops.h>
#include <device.h>
#include <fs/vfs_impl.h>
#include <kstdio.h>
#include <mm/kmalloc.h>

#define EXT2_DIRECT_BLOCKS 12
#define EXT2_SB_OFFSET 1024
#define EXT2_BGD_OFFSET 2048

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

static void calc_block_group(const struct ext2_fs *fs, blkcnt_t block,
                             u32 *group, u32 *in_group) {
    *group = block / fs->sb.s_blocks_per_group;
    *in_group = block % fs->sb.s_blocks_per_group;
}

static void calc_ino_group(const struct ext2_fs *fs, ino_t ino, u32 *group,
                           u32 *in_group) {
    assert(ino);
    --ino;
    *group = ino / fs->sb.s_inodes_per_group;
    *in_group = ino % fs->sb.s_inodes_per_group;
}

static off_t calc_inode_phys_offset(const struct ext2_fs *fs, ino_t ino) {
    u32 ino_group;
    blkcnt_t ino_in_group;
    calc_ino_group(fs, ino, &ino_group, &ino_in_group);
    expects(ino_group < fs->bgds_count);
    u32 ino_table = fs->bgds[ino_group].bg_inode_table;
    u64 ino_offset = ino_in_group * sizeof(struct ext2_inode);
    return ino_table * fs->block_size + ino_offset;
}

static void read_inode(struct superblock *fs, struct ext2_inode *inode,
                       ino_t ino) {
    expects(ino);
    struct ext2_fs *ext2 = fs->private;
    off_t inode_offset = calc_inode_phys_offset(ext2, ino);
    blk_read(fs->dev, inode_offset, inode, sizeof(*inode));
}

static void write_inode(struct superblock *fs, const struct ext2_inode *inode,
                        ino_t ino) {
    expects(ino);
    struct ext2_fs *ext2 = fs->private;
    off_t inode_offset = calc_inode_phys_offset(ext2, ino);
    blk_write(fs->dev, inode_offset, inode, sizeof(*inode));
}

static void read_superblock(struct superblock *fs, struct ext2_sb *sb) {
    blk_read(fs->dev, EXT2_SB_OFFSET, sb, sizeof(*sb));
}

static void sync_superblock(struct superblock *fs) {
    struct ext2_fs *ext2 = fs->private;
    blk_write(fs->dev, EXT2_SB_OFFSET, &ext2->sb, sizeof(ext2->sb));
}

// note: need to provide is_dir because ext2 block group tracks directory count
static ssize_t alloc_ino(struct superblock *sb, int is_dir) {
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
    blk_read(sb->dev, desc->bg_inode_bitmap * ext2->block_size, bitmap,
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

    desc->bg_used_dirs_count += !!is_dir;
    set_bit(found, bitmap);

    blk_write(sb->dev, desc->bg_inode_bitmap * ext2->block_size, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);

    return bgdi * ext2->sb.s_inodes_per_group + found;
}

static void free_ino(struct superblock *sb, ino_t ino, int is_dir) {
    struct ext2_fs *ext2 = sb->private;
    u32 ino_group, ino_in_group;
    calc_ino_group(ext2, ino, &ino_group, &ino_in_group);
    expects(ino_group < ext2->bgds_count);

    struct ext2_group_desc *group = ext2->bgds + ino_group;
    u64 bitmap[ext2->group_inode_bitmap_size];
    blk_read(sb->dev, group->bg_inode_bitmap * ext2->block_size, bitmap,
             sizeof(bitmap));

    if (!test_bit(ino_in_group, bitmap))
        ext2_error(sb, "corrupted inode bitmap");

    clear_bit(ino_in_group, bitmap);
    ++group->bg_free_inodes_count;
    if (is_dir && !group->bg_used_dirs_count)
        ext2_error(sb, "corrupted block group used dir count");

    group->bg_used_dirs_count -= is_dir;
    ++ext2->sb.s_free_inode_count;

    blk_write(sb->dev, group->bg_inode_bitmap * ext2->block_size, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);
}

static ssize_t alloc_block(struct superblock *sb) {
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
    blk_read(sb->dev, desc->bg_block_bitmap * ext2->block_size, bitmap,
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

    blk_write(sb->dev, desc->bg_block_bitmap * ext2->block_size, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);

    return bgdi * ext2->sb.s_blocks_per_group + found - 1;
}

static void free_block(struct superblock *sb, blkcnt_t block) {
    struct ext2_fs *fs = sb->private;
    u32 block_group, block_in_group;
    calc_block_group(fs, block, &block_group, &block_in_group);

    expects(block_group < fs->bgds_count);
    struct ext2_group_desc *group = fs->bgds + block_group;
    u64 bitmap[fs->group_block_bitmap_size];
    blk_read(sb->dev, group->bg_block_bitmap * fs->block_size, bitmap,
             sizeof(bitmap));

    if (!test_bit(block_in_group, bitmap))
        ext2_error(sb, "corrupted block bitmap");

    clear_bit(block_in_group, bitmap);
    ++group->bg_free_blocks_count;
    ++fs->sb.s_free_block_count;

    blk_write(sb->dev, group->bg_block_bitmap * fs->block_size, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);
}

int ext2_mount(struct superblock *sb) {
    struct ext2_fs *fs = sb->private;
    int rc = 0;

    read_superblock(sb, &fs->sb);

    // TODO: Actual count should depend on filesystem partition size
    // roughly partition_size_in_blocks / (8 * block_size)
    size_t bgds_count = 1;
    size_t bgds_size = bgds_count * sizeof(struct ext2_group_desc);
    struct ext2_group_desc *bgds = kmalloc(bgds_size);
    if (!bgds) return -ENOMEM;
    blk_read(sb->dev, EXT2_BGD_OFFSET, bgds, bgds_size);

    fs->bgds_count = bgds_count;
    fs->bgds = bgds;

    fs->block_size = 1 << fs->sb.s_log_block_size;
    fs->inodes_per_block = fs->block_size / sizeof(struct ext2_inode);
    fs->group_inode_bitmap_size = BITS_TO_BITMAP(fs->sb.s_inodes_per_group);
    fs->group_inode_bitmap_size = BITS_TO_BITMAP(fs->sb.s_blocks_per_group);
    fs->blocks_per_inderect_block = fs->block_size / sizeof(u32);
    fs->first_2lev_inderect_block = 12 + fs->blocks_per_inderect_block;
    fs->first_3lev_inderect_block =
        fs->first_2lev_inderect_block +
        fs->blocks_per_inderect_block * fs->blocks_per_inderect_block;

    read_inode(sb, &fs->root_inode, EXT2_ROOT_INO);

    return rc;
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

const struct file_ops ops = {.lseek = generic_lseek,
                             .read = ext2_read,
                             .write = ext2_write,
                             .open = ext2_open,
                             .release = ext2_release,
                             .readdir = ext2_readdir};
