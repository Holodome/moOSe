#include <assert.h>
#include <bitops.h>
#include <blk_device.h>
#include <endian.h>
#include <errno.h>
#include <fs/ext2.h>
#include <kstdio.h>
#include <mm/kmalloc.h>
#include <string.h>

#define EXT2_DIRECT_BLOCKS 12
#define EXT2_SB_OFFSET 1024

// The first few entries of the inode tables are reserved. In revision 0 there
// are 11 entries reserved while in revision 1 (EXT2_DYNAMIC_REV) and later the
// number of reserved inodes entries is specified in the s_first_ino of the
// superblock structure
#define EXT2_FIRST_INO 11

// superblock state
#define EXT2_VALID_FS 1
#define EXT2_ERROR_FS 2

// superblock errors
#define EXT2_ERRORS_CONTINUE 1 // continue as nothing handled
#define EXT2_ERRORS_RO 2       // remount read-only
#define EXT2_ERRORS_PANIC 3    // cause kernel panic

// superblock os
#define EXT2_OS_MOOSE 100

// dentry file type
#define EXT2_FT_UNKNOWN 0  // Unknown File Type
#define EXT2_FT_REG_FILE 1 // Regular File
#define EXT2_FT_DIR 2      // Directory File
#define EXT2_FT_CHRDEV 3   // Character Device
#define EXT2_FT_BLKDEV 4   // Block Device
#define EXT2_FT_FIFO 5     // Buffer File
#define EXT2_FT_SOCK 6     // Socket File
#define EXT2_FT_SYMLINK 7  // Symbolic Link

#define EXT2_BAD_INO 1         // bad blocks inode
#define EXT2_ROOT_INO 2        // root directory inode
#define EXT2_ACL_IDX_INO 3     // ACL index inode (deprecated?)
#define EXT2_ACL_DATA_INO 4    // ACL data inode (deprecated?)
#define EXT2_BOOT_LOADER_INO 5 // boot loader inode
#define EXT2_UNDEL_DIR_INO 6   //

#define EXT2_SUPER_MAGIC 0xEF53

#define EXT2_NAME_LEN 255

// file format
#define EXT2_S_IFSOCK 0xC000 // socket
#define EXT2_S_IFLNK 0xA000  // symbolic link
#define EXT2_S_IFREG 0x8000  // regular file
#define EXT2_S_IFBLK 0x6000  // block device
#define EXT2_S_IFDIR 0x4000  // directory
#define EXT2_S_IFCHR 0x2000  // character device
#define EXT2_S_IFIFO 0x1000  // fifo
// process execution user/group override --
#define EXT2_S_ISUID 0x0800 // Set process User ID
#define EXT2_S_ISGID 0x0400 // Set process Group ID
#define EXT2_S_ISVTX 0x0200 // sticky bit
// access rights
#define EXT2_S_IRUSR 0x0100 // user read
#define EXT2_S_IWUSR 0x0080 // user write
#define EXT2_S_IXUSR 0x0040 // user execute
#define EXT2_S_IRGRP 0x0020 // group read
#define EXT2_S_IWGRP 0x0010 // group write
#define EXT2_S_IXGRP 0x0008 // group execute
#define EXT2_S_IROTH 0x0004 // others read
#define EXT2_S_IWOTH 0x0002 // others write
#define EXT2_S_IXOTH 0x0001 // others execute

// superblock
struct ext2_sb {
    u32 s_inode_count;
    u32 s_block_count;
    u32 s_reserved_block_count;
    u32 s_free_block_count;
    u32 s_free_inode_count;
    u32 s_first_data_block;
    u32 s_log_block_size;
    u32 s_log_frag_size;
    u32 s_blocks_per_group;
    u32 s_fragments_per_group;
    u32 s_inodes_per_group;
    u32 s_mtime;
    u32 s_wtime;
    u16 s_mnt_count;
    u16 s_max_mnt_count;
    u16 s_magic;
    u16 s_state;
    u16 s_errors;
    u16 s_minor_rev_level;
    u32 s_lastcheck;
    u32 s_checkinterval;
    u32 s_creator_os;
    u32 s_rev_level;
    u16 s_def_resuid;
    u16 s_def_resgid;
};

static_assert(sizeof(struct ext2_sb) == 84);

struct ext2_group_desc {
    u32 bg_block_bitmap;
    u32 bg_inode_bitmap;
    u32 bg_inode_table;
    u16 bg_free_blocks_count;
    u16 bg_free_inodes_count;
    u16 bg_used_dirs_count;
    u16 bg_pad;
    u32 reserved[3];
};

static_assert(sizeof(struct ext2_group_desc) == 32);

struct ext2_inode {
    u16 i_mode;
    u16 i_uid;
    u32 i_size;
    u32 i_atime;
    u32 i_ctime;
    u32 i_mtime;
    u32 i_dtime;
    u16 i_gid;
    u16 i_links_count;
    u32 i_blocks;
    u32 i_flags;
    u32 i_osd1;
    u32 i_block[15];
    u32 i_generation;
    u32 i_file_acl;
    u32 i_dir_acl;
    u32 i_faddr;
    u32 i_osd2[3];
};

static_assert(sizeof(struct ext2_inode) == 128);

struct ext2_dentry {
    u32 inode;
    u16 rec_len;
    u8 name_len;
    u8 file_type;
};

static_assert(sizeof(struct ext2_dentry) == 8);

struct ext2_dentry1 {
    u32 inode;
    u16 rec_len;
    u8 name_len;
    u8 file_type;
    char name[EXT2_NAME_LEN];
};

struct ext2_fs {
    struct ext2_sb sb;

    size_t bgds_count;
    struct ext2_group_desc *bgds;

    struct ext2_inode root_inode;

    u32 inodes_per_block;
    u32 group_inode_bitmap_size;
    u32 group_block_bitmap_size;

    u32 blocks_per_inderect_block;
    u32 first_2lev_inderect_block;
    u32 first_3lev_inderect_block;
};

static ssize_t ext2_write(struct file *filp, const void *buf, size_t count) {
    (void)filp;
    (void)buf;
    (void)count;
    return 0;
}
static ssize_t ext2_read(struct file *filp, void *buf, size_t count) {
    (void)filp;
    (void)buf;
    (void)count;
    return 0;
}
static void ext2_release_sb(struct superblock *sb);
static void ext2_free_inode(struct inode *inode);
static int ext2_readdir(struct file *filp, struct dentry *entry);
static const struct sb_ops sb_ops = {.release_sb = ext2_release_sb};
static const struct inode_ops inode_ops = {.free = ext2_free_inode};
static const struct file_ops file_ops = {.lseek = generic_lseek,
                                         .read = ext2_read,
                                         .write = ext2_write,
                                         .readdir = ext2_readdir};

static void sync_superblock(struct superblock *);

__used static void print_raw_inode(const struct ext2_inode *inode) {
    kprintf("ext2_inode\n");
    kprintf(" mode=%lu uid=%lu gid=%lu\n", (unsigned long)inode->i_mode,
            (unsigned long)inode->i_uid, (unsigned long)inode->i_gid);
    kprintf(" size=%ld nlink=%ld blkcnt=%ld\n", (unsigned long)inode->i_size,

            (unsigned long)inode->i_links_count,
            (unsigned long)inode->i_blocks);
}

/* static void print_bgd(const struct ext2_group_desc *d) { */
/*     kprintf("gd bbit=%u ibit=%u it=%u\n", d->bg_block_bitmap, */
/*             d->bg_inode_bitmap, d->bg_inode_table); */
/* } */

__used static void print_dentry(const struct ext2_dentry *entry) {
    kprintf("de ino=%u name_len=%u rec_len=%u\n", entry->inode, entry->name_len,
            entry->rec_len);
}

static struct ext2_fs *sb_ext2(struct superblock *sb) {
    return sb->private;
}

#define ext2_error(_sb, _fmt, ...)                                             \
    ext2_error_(_sb, __PRETTY_FUNCTION__, _fmt, ##__VA_ARGS__)
static void ext2_error_(struct superblock *sb, const char *func_name,
                        const char *fmt, ...) {
    struct ext2_fs *ext2 = sb_ext2(sb);
    ext2->sb.s_state |= EXT2_ERROR_FS;
    sync_superblock(sb);

    va_list args;
    va_start(args, fmt);
    kprintf("ext2 error in %s: ", func_name);
    kvprintf(fmt, args);
    kprintf("\n");
    va_end(args);
}

static struct ktimespec ext2_to_timespec(u32 raw) {
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

static void sync_superblock(struct superblock *sb) {
    struct ext2_fs *ext2 = sb_ext2(sb);
    blk_write(sb->dev, EXT2_SB_OFFSET, &ext2->sb, sizeof(ext2->sb));
}

// note: need to provide is_dir because ext2 block group tracks directory
// count
__used static ssize_t ext2_alloc_ino(struct superblock *sb, int is_dir) {
    struct ext2_fs *ext2 = sb_ext2(sb);

    size_t bgdi;
    struct ext2_group_desc *desc = NULL;
    for (bgdi = 0; bgdi < ext2->bgds_count && !desc; ++bgdi) {
        struct ext2_group_desc *test = ext2->bgds + bgdi;
        if (!test->bg_free_inodes_count)
            continue;

        desc = test;
    }

    if (desc == NULL)
        return -ENOSPC;

    u64 bitmap[ext2->group_inode_bitmap_size];
    blk_read(sb->dev, desc->bg_inode_bitmap << sb->blk_sz_bits, bitmap,
             sizeof(bitmap));

    u64 found = bitmap_first_clear(bitmap, ext2->sb.s_inodes_per_group);
    if (!found)
        ext2_error(sb, "corrupted inode bitmap");

    if (desc->bg_free_inodes_count)
        --desc->bg_free_inodes_count;
    else
        ext2_error(sb, "currupted block group free inode count");

    if (ext2->sb.s_free_inode_count)
        --ext2->sb.s_free_inode_count;
    else
        ext2_error(sb, "corrupted superblock free inode count");

    if (is_dir)
        --desc->bg_used_dirs_count;
    set_bit(found, bitmap);

    blk_write(sb->dev, desc->bg_inode_bitmap << sb->blk_sz_bits, bitmap,
              sizeof(bitmap));
    sync_superblock(sb);

    return bgdi * ext2->sb.s_inodes_per_group + found;
}

static void ext2_free_ino(struct superblock *sb, ino_t ino, int is_dir) {
    struct ext2_fs *ext2 = sb_ext2(sb);
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

__used static ssize_t ext2_alloc_block(struct superblock *sb) {
    struct ext2_fs *ext2 = sb_ext2(sb);
    size_t bgdi;
    struct ext2_group_desc *desc = NULL;
    for (bgdi = 0; bgdi < ext2->bgds_count && !desc; ++bgdi) {
        struct ext2_group_desc *test = ext2->bgds + bgdi;
        if (!test->bg_free_blocks_count)
            continue;

        desc = test;
    }

    if (desc == NULL)
        return -ENOSPC;

    u64 bitmap[ext2->group_block_bitmap_size];
    blk_read(sb->dev, desc->bg_block_bitmap << sb->blk_sz_bits, bitmap,
             sizeof(bitmap));

    u64 found = bitmap_first_clear(bitmap, ext2->sb.s_blocks_per_group);
    if (!found)
        ext2_error(sb, "corrupted block bitmap");

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

__used static void ext2_free_block(struct superblock *sb, blkcnt_t block) {
    struct ext2_fs *fs = sb_ext2(sb);
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

static void ext2_get_raw_inode(struct superblock *sb, ino_t ino,
                               struct ext2_inode *ei) {
    struct ext2_fs *ext2 = sb_ext2(sb);

    u32 ino_group, ino_in_group;
    calc_ino_group(ext2, ino, &ino_group, &ino_in_group);
    expects(ino_group < ext2->bgds_count);
    struct ext2_group_desc *bgd = ext2->bgds + ino_group;

    off_t inode_offset = (bgd->bg_inode_table << sb->blk_sz_bits) +
                         ino_in_group * sizeof(struct ext2_inode);

    blk_read(sb->dev, inode_offset, ei, sizeof(*ei));
}

static struct inode *ext2_iget(struct superblock *sb, ino_t ino) {
    struct ext2_inode ei;
    ext2_get_raw_inode(sb, ino, &ei);

    struct inode *inode = alloc_inode();
    if (!inode)
        return ERR_PTR(-ENOMEM);

    inode->ino = ino;
    inode->mode = ei.i_mode;
    inode->uid = ei.i_uid;
    inode->gid = ei.i_gid;
    inode->size = ei.i_size;
    inode->nlink = ei.i_links_count;
    inode->block_count = ei.i_blocks;
    inode->atime = ext2_to_timespec(ei.i_atime);
    inode->mtime = ext2_to_timespec(ei.i_mtime);
    inode->ctime = ext2_to_timespec(ei.i_ctime);

    inode->ops = &inode_ops;
    inode->file_ops = &file_ops;
    inode->sb = sb;

    return inode;
}

__used static void ext2_write_inode(struct inode *inode) {
    struct superblock *sb = inode->sb;
    struct ext2_fs *ext2 = sb_ext2(sb);

    ino_t ino = inode->ino;
    u32 ino_group, ino_in_group;
    calc_ino_group(ext2, ino, &ino_group, &ino_in_group);
    expects(ino_group < ext2->bgds_count);
    struct ext2_group_desc *bgd = ext2->bgds + ino_group;

    off_t inode_offset = (bgd->bg_inode_table << sb->blk_sz_bits) +
                         ino_in_group * sizeof(struct ext2_inode);
    struct ext2_inode ei = {0};
    ext2_get_raw_inode(sb, ino, &ei);

    ei.i_mode = inode->mode;
    ei.i_uid = inode->uid;
    ei.i_gid = inode->gid;
    ei.i_size = inode->size;
    ei.i_links_count = inode->nlink;
    ei.i_blocks = inode->block_count;
    ei.i_atime = inode->atime.tv_sec;
    ei.i_mtime = inode->mtime.tv_sec;
    ei.i_ctime = inode->ctime.tv_sec;
    // NOTE: We assume that other inode fields (i_block particullary) are
    // always synced
    blk_write(sb->dev, inode_offset, &ei, sizeof(ei));
}

static int ext2_do_mount(struct superblock *sb) {
    struct ext2_fs *ext2 = sb_ext2(sb);
    read_superblock(sb, &ext2->sb);

    u32 blk_sz = 1024 << ext2->sb.s_log_block_size;
    size_t bgds_count = 1;
    expects(bgds_count != 0);
    size_t bgds_size = bgds_count * sizeof(struct ext2_group_desc);
    struct ext2_group_desc *bgds = kmalloc(bgds_size);
    if (!bgds)
        return -ENOMEM;

    size_t bgds_loc = 1 * blk_sz;
    blk_read(sb->dev, bgds_loc, bgds, bgds_size);

    ext2->bgds_count = bgds_count;
    ext2->bgds = bgds;

    u32 inodes_per_group = ext2->sb.s_inodes_per_group;
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
    struct ext2_fs *ext2 = sb_ext2(sb);
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
    if (to_read > count)
        to_read = count;

    off_t phys_offset = ext2_get_disk_blk(inode, sb, current_block)
                        << sb->blk_sz_bits;
    blk_read(sb->dev, phys_offset + cursor_in_block, buf, to_read);
    return to_read;
}

#if 0
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
#endif

int ext2_mount(struct superblock *sb) {
    struct ext2_fs *ext2 = kzalloc(sizeof(*ext2));
    if (!ext2)
        return -ENOMEM;
    sb->private = ext2;
    int result = ext2_do_mount(sb);
    if (result) {
        kfree(ext2);
        return result;
    }

    sb->ops = &sb_ops;
    sb->blk_sz_bits = 10 + ext2->sb.s_log_block_size;
    sb->blk_sz = 1 << sb->blk_sz_bits;

    struct inode *root_inode = ext2_iget(sb, EXT2_ROOT_INO);
    if (IS_PTR_ERR(root_inode)) {
        ext2_release_sb(sb);
        return PTR_ERR(root_inode);
    }
    struct dentry *root = create_root_dentry();
    if (!root) {
        release_inode(root_inode);
        ext2_release_sb(sb);
        return -ENOMEM;
    }
    init_dentry(root, root_inode);
    sb->root = root;

    return 0;
}

// 0 - success
// 1 - no entries left
static int ext2_read_dentry_at(struct ext2_inode *ei, struct superblock *sb,
                               off_t *offset, struct ext2_dentry1 *ed) {
    if (*offset >= ei->i_size)
        return 1;
    size_t read =
        ext2_read_in_block(ei, sb, *offset, ed, sizeof(struct ext2_dentry));
    if (read != sizeof(struct ext2_dentry)) {
        kprintf("ext2: directory entry does not span block");
        return -EIO;
    }
    size_t name_len = ed->name_len;
    if (name_len > EXT2_NAME_LEN) {
        kprintf("ext2: name length is wrong %zu\n", name_len);
        return -ENAMETOOLONG;
    }
    read = ext2_read_in_block(ei, sb, *offset + sizeof(struct ext2_dentry),
                              ed->name, ed->name_len);
    if (read != ed->name_len) {
        kprintf("ext2: directory entry does not span block");
        return -EIO;
    }
    ed->name[ed->name_len] = '\0';
    *offset += ed->rec_len;
    return 0;
}

static int ext2_readdir(struct file *dir, struct dentry *entry) {
    struct inode *inode = dir->dentry->inode;
    expects(S_ISDIR(inode->mode));
    struct superblock *sb = inode->sb;
    if (dir->offset >= inode->size)
        return -ENOENT;
    struct ext2_inode ei;
    ext2_get_raw_inode(sb, inode->ino, &ei);
    struct ext2_dentry1 ed;
    // TODO: If read_dentry_at fails with error we should try to read next
    // directory entry
    off_t new_offset = dir->offset;
    int read_result = ext2_read_dentry_at(&ei, sb, &new_offset, &ed);
    if (read_result)
        return read_result < 0 ? read_result : -ENOENT;

    // TODO: 0 means that dentry is not allocated, we should read next
    if (ed.inode == 0)
        return -ENOENT;

    // TODO: ed.name is not zero-terminated!!!
    if (dentry_set_name(entry, ed.name))
        return -ENOMEM;

    struct inode *dentry_inode = ext2_iget(sb, ed.inode);
    if (IS_PTR_ERR(dentry_inode))
        return PTR_ERR(dentry_inode);

    init_dentry(entry, dentry_inode);
    dir->offset = new_offset;

    return 0;
}

static void ext2_release_sb(struct superblock *sb) {
    struct ext2_fs *ext2 = sb_ext2(sb);
    kfree(ext2->bgds);
    kfree(ext2);
}

static void ext2_free_inode(struct inode *inode) {
    struct superblock *sb = inode->sb;
    ext2_free_ino(sb, inode->ino, S_ISDIR(inode->mode));
}
