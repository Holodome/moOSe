#include <fs/ext2.h>

#include <assert.h>
#include <bitops.h>
#include <device.h>
#include <fs/vfs_impl.h>
#include <kstdio.h>
#include <mm/kmalloc.h>

#define EXT2_SB_OFFSET 1024
#define EXT2_BGD_OFFSET 2048

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
    assert(ino_group < fs->bgds_count);
    u32 ino_table = fs->bgds[ino_group].bg_inode_table;
    u64 ino_offset = ino_in_group * sizeof(struct ext2_inode);
    return ino_table * fs->block_size + ino_offset;
}

static void read_inode(struct superblock *fs, struct ext2_inode *inode,
                       ino_t ino) {
    struct ext2_fs *ext2 = fs->private;
    assert(ino);
    off_t inode_offset = calc_inode_phys_offset(ext2, ino);
    blk_read(fs->dev, inode_offset, inode, sizeof(*inode));
}

static void write_inode(struct superblock *fs, const struct ext2_inode *inode,
                        ino_t ino) {
    struct ext2_fs *ext2 = fs->private;
    assert(ino);
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
static ssize_t alloc_ino(struct superblock *fs, int is_dir) {
    struct ext2_fs *ext2 = fs->private;
    struct ext2_sb *sb = &ext2->sb;

    size_t bgdi;
    struct ext2_group_desc *desc = NULL;
    for (bgdi = 0; bgdi < ext2->bgds_count && !desc; ++bgdi) {
        struct ext2_group_desc *test = ext2->bgds + bgdi;
        if (!test->bg_free_inodes_count) continue;

        desc = test;
    }

    if (desc == NULL) return -ENOSPC;

    u64 bitmap[ext2->group_inode_bitmap_size];
    blk_read(fs->dev, desc->bg_inode_bitmap * ext2->block_size, bitmap,
             sizeof(bitmap));

    u64 found = bitmap_first_clear(bitmap, sb->s_inodes_per_group);
    assert(found);

    assert(desc->bg_free_inodes_count);
    --desc->bg_free_inodes_count;
    assert(sb->s_free_inode_count);
    --sb->s_free_inode_count;
    desc->bg_used_dirs_count += !!is_dir;
    set_bit(found, bitmap);

    blk_write(fs->dev, desc->bg_inode_bitmap * ext2->block_size, bitmap,
              sizeof(bitmap));
    sync_superblock(fs);

    return bgdi * sb->s_inodes_per_group + found;
}

static void free_ino(struct superblock *sb, ino_t ino, int is_dir) {
    struct ext2_fs *ext2 = sb->private;
    u32 ino_group, ino_in_group;
    calc_ino_group(ext2, ino, &ino_group, &ino_in_group);
    assert(ino_group < ext2->bgds_count);
    struct ext2_group_desc *group = ext2->bgds + ino_group;
    u64 bitmap[ext2->group_inode_bitmap_size];
    blk_read(sb->dev, group->bg_inode_bitmap * ext2->block_size, bitmap,
             sizeof(bitmap));

    assert(test_bit(ino_in_group, bitmap));

    clear_bit(ino_in_group, bitmap);
    ++group->bg_free_inodes_count;
    assert(!is_dir || group->bg_used_dirs_count);
    group->bg_used_dirs_count -= is_dir;
    ++ext2->sb.s_free_inode_count;

    blk_write(sb->dev, group->bg_inode_bitmap * ext2->block_size, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);
}

static ssize_t alloc_block(struct superblock *sb) {
    struct ext2_fs *fs = sb->private;
    size_t bgdi;
    struct ext2_group_desc *desc = NULL;
    for (bgdi = 0; bgdi < fs->bgds_count && !desc; ++bgdi) {
        struct ext2_group_desc *test = fs->bgds + bgdi;
        if (!test->bg_free_blocks_count) continue;

        desc = test;
    }

    if (desc == NULL) return -ENOSPC;

    u64 bitmap[fs->group_block_bitmap_size];
    blk_read(sb->dev, desc->bg_block_bitmap * fs->block_size, bitmap,
             sizeof(bitmap));

    u64 found = bitmap_first_clear(bitmap, fs->sb.s_blocks_per_group);
    assert(found);

    assert(desc->bg_free_blocks_count);
    --desc->bg_free_blocks_count;
    assert(fs->sb.s_free_block_count);
    --fs->sb.s_free_block_count;
    set_bit(found, bitmap);

    blk_write(sb->dev, desc->bg_block_bitmap * fs->block_size, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);

    return bgdi * fs->sb.s_blocks_per_group + found - 1;
}

static void free_block(struct superblock *sb, blkcnt_t block) {
    struct ext2_fs *fs = sb->private;
    u32 block_group, block_in_group;
    calc_block_group(fs, block, &block_group, &block_in_group);
    assert(block_group < fs->bgds_count);
    struct ext2_group_desc *group = fs->bgds + block_group;
    u64 bitmap[fs->group_block_bitmap_size];
    blk_read(sb->dev, group->bg_block_bitmap * fs->block_size, bitmap,
             sizeof(bitmap));

    assert(test_bit(block_in_group, bitmap));

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

    read_inode(sb, &fs->root_inode, EXT2_ROOT_INO);

    return rc;
}

static int read_in_block(struct ext2_inode *inode, struct superblock *sb,
                         off_t *at, void *buf, size_t *count) {
    struct ext2_fs *ext2 = sb->private;
    off_t cursor = *at;
    off_t cursor_in_block = cursor % sb->blk_sz;
    off_t current_block = cursor / sb->blk_sz;
    u32 to_read = __block_end(sb, cursor) - cursor;
    if (to_read > *count) to_read = *count - cursor;

    if (current_block < 12) {
        blkcnt_t block = inode->i_block[current_block];
        assert(block);
        off_t phys_offset = block * sb->blk_sz;
        blk_read(sb->dev, phys_offset + cursor_in_block, buf, to_read);
    } else if (current_block < ext2->first_2lev_inderect_block) {
        off_t table_offset = inode->i_block[12] * sb->blk_sz;
        blkcnt_t a = current_block - 12;
        u32 block;
        blk_read(sb->dev, table_offset + sizeof(u32) * a, &block,
                 sizeof(block));

        off_t phys_offset = block * sb->blk_sz;
        blk_read(sb->dev, phys_offset + cursor_in_block, buf, to_read);
    } else if (current_block < ext2->first_3lev_inderect_block) {
        off_t l1_table_offset = inode->i_block[13] * sb->blk_sz;
        blkcnt_t a = current_block - ext2->first_2lev_inderect_block;
        blkcnt_t l1_idx = a / ext2->blocks_per_inderect_block;
        blkcnt_t l2_idx = a % ext2->blocks_per_inderect_block;

        u32 l1;
        blk_read(sb->dev, l1_table_offset + l1_idx * sizeof(u32), &l1,
                 sizeof(l1));
        off_t l2_table_offset = l1 * sb->blk_sz;
        u32 l2;
        blk_read(sb->dev, l2_table_offset + l2_idx * sizeof(u32), &l2,
                 sizeof(l2));

        off_t phys_offset = l2 * sb->blk_sz;
        blk_read(sb->dev, phys_offset + cursor_in_block, buf, to_read);
    } else {
        off_t l1_table_offset = inode->i_block[14] * sb->blk_sz;
        blkcnt_t a = current_block - ext2->first_3lev_inderect_block;
        blkcnt_t l1_idx = a / (ext2->blocks_per_inderect_block *
                               ext2->blocks_per_inderect_block);
        a = a %
            (ext2->blocks_per_inderect_block * ext2->blocks_per_inderect_block);
        blkcnt_t l2_idx = a / ext2->blocks_per_inderect_block;
        blkcnt_t l3_idx = a % ext2->blocks_per_inderect_block;

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

        off_t phys_offset = l3 << sb->blk_sz_bits;
        blk_read(sb->dev, phys_offset + cursor_in_block, buf, to_read);
    }

    *at += to_read;
    *count += to_read;

    return 0;
}

ssize_t ext2_read(struct file *filp, void *buf, size_t count) {
    struct inode *inode = filp->dentry->inode;
    struct superblock *sb = inode->sb;

    struct ext2_inode *einode = inode->private;
    expects(inode != NULL);

    off_t cursor = filp->offset;

    //
}

ssize_t ext2_write(struct file *, const void *, size_t);
int ext2_open(struct inode *, struct file *);
int ext2_release(struct inode *, struct file *);
int ext2_readdir(struct file *, struct dentry *);

const struct file_ops ops = {.lseek = generic_lseek,
                             .read = ext2_read,
                             .write = ext2_write,
                             .open = ext2_open,
                             .release = ext2_release,
                             .readdir = ext2_readdir};
