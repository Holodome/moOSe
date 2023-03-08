#include <fs/ext2.h>

#include <assert.h>
#include <bitops.h>
#include <device.h>
#include <errno.h>
#include <kstdio.h>
#include <mm/kmalloc.h>

#define EXT2_SB_OFFSET 1024
#define EXT2_BGD_OFFSET 2048

static u32 calculate_inode_phys_offset(struct ext2_fs *fs, u32 ino) {
    assert(ino);
    --ino;
    u32 ino_group = ino / fs->sb.s_inodes_per_group;
    u32 ino_in_group = ino % fs->sb.s_inodes_per_group;
    u32 ino_block_in_group = ino_in_group / fs->inodes_per_block;
    u32 ino_in_block = ino_in_group % fs->inodes_per_block;
    assert(ino_group < fs->bgds_count);
    u32 ino_block = fs->bgds[ino_group].bg_inode_table + ino_block_in_group;
    u32 ino_offset = ino_in_block * sizeof(struct ext2_inode);

    return ino_block * fs->block_size + ino_offset;
}

static int read_inode(struct ext2_fs *fs, struct ext2_inode *inode, u32 ino) {
    assert(ino);
    u32 inode_offset = calculate_inode_phys_offset(fs, ino);
    return seek_read(fs->dev, inode_offset, inode, sizeof(*inode));
}

static int write_inode(struct ext2_fs *fs, const struct ext2_inode *inode,
                       u32 ino) {
    assert(ino);
    u32 inode_offset = calculate_inode_phys_offset(fs, ino);
    return seek_write(fs->dev, inode_offset, inode, sizeof(*inode));
}

static int read_superblock(struct ext2_fs *fs, struct ext2_sb *sb) {
    return seek_read(fs->dev, EXT2_SB_OFFSET, sb, sizeof(*sb));
}

// note: need to provide is_dir because ext2 block group tracks directory count
static ssize_t alloc_ino(struct ext2_fs *fs, int is_dir) {
    size_t bgdi;
    struct ext2_group_desc *desc = NULL;
    for (bgdi = 0; bgdi < fs->bgds_count && !desc; ++bgdi) {
        struct ext2_group_desc *test = fs->bgds + bgdi;
        if (!test->bg_free_inodes_count) continue;

        desc = test;
    }

    if (desc == NULL) return -ENOSPC;

    u64 bitmap[fs->group_inode_bitmap_size];
    if (seek_read(fs->dev, desc->bg_inode_bitmap * fs->block_size, bitmap,
                  sizeof(bitmap)) < 0)
        return -EIO;

    u64 found = 0;
    for (size_t i = 0; i < fs->sb.s_inodes_per_group && !found;
         i += BITMAP_STRIDE) {
        u64 biti = bit_scan_forward(bitmap[i / BITMAP_STRIDE]);
        if (biti) found = i + biti - 1;
    }

    if (!found) panic("Corrupted filestem");

    assert(desc->bg_free_inodes_count);
    --desc->bg_free_inodes_count;
    assert(fs->sb.s_free_inode_count);
    --fs->sb.s_free_inode_count;
    desc->bg_used_dirs_count += !!is_dir;
    set_bit(found, bitmap);

    if (seek_write(fs->dev, desc->bg_inode_bitmap * fs->block_size, bitmap,
                   sizeof(bitmap)) < 0)
        return -EIO;

    u32 ino = bgdi * fs->sb.s_inodes_per_group + found + 1;
    return ino;
}

int ext2_mount(struct ext2_fs *fs) {
    int rc = 0;

    if (read_superblock(fs, &fs->sb) < 0) return -EIO;

    if (lseek(fs->dev, EXT2_BGD_OFFSET, SEEK_SET) < 0) return -EIO;

    // TODO: Actual count should depend on filessytem partition size
    // roughly partition_size_in_blocks / (8 * block_size)
    size_t bgds_count = 1;
    size_t bgds_size = bgds_count * sizeof(struct ext2_group_desc);
    struct ext2_group_desc *bgds = kmalloc(bgds_size);
    for (size_t i = 0; i < bgds_count; ++i) {
        if (read(fs->dev, bgds + i, sizeof(*bgds)) != sizeof(*bgds)) {
            rc = -EIO;
            goto free_bgds;
        }
    }

    fs->bgds_count = bgds_count;
    fs->bgds = bgds;

    fs->block_size = 1 << fs->sb.s_log_block_size;
    fs->inodes_per_block = fs->block_size / sizeof(struct ext2_inode);
    fs->group_inode_bitmap_size = BITS_TO_BITMAP(fs->sb.s_inodes_per_group);
    fs->group_inode_bitmap_size = BITS_TO_BITMAP(fs->sb.s_blocks_per_group);

    if (read_inode(fs, &fs->root_inode, EXT2_ROOT_INO) < 0) {
        rc = -EIO;
        goto free_bgds;
    }

    return rc;
free_bgds:
    kfree(bgds);

    return rc;
}
