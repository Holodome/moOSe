#include <fs/ramfs.h>

#include <mm/kmalloc.h>
#include <string.h>

#define RAMFS_BLOCK_SIZE 4096
#define RAMFS_BLOCK_SIZE_BITS 12

struct ramfs_block {
    struct list_head list;
    char data[RAMFS_BLOCK_SIZE];
};

struct ramfs_inode {
    size_t block_count;
    struct list_head block_list;
};

struct ramfs {
    ino_t inode_counter;
    struct list_head block_freelist;
};

static struct ramfs *sb_ram(const struct superblock *sb) { return sb->private; }
static struct ramfs_inode *i_ram(const struct inode *inode) {
    return inode->private;
}

static struct ramfs_block *ramfs_get_empty_block(struct ramfs *fs) {
    struct ramfs_block *block =
        list_first_or_null(&fs->block_freelist, struct ramfs_block, list);
    if (!block)
        block = kmalloc(sizeof(*block));
    else
        list_remove(&block->list);
    return block;
}

static void ramfs_release_sb(struct superblock *sb) {
    struct ramfs *fs = sb_ram(sb);
    struct ramfs_block *block, *temp;
    list_for_each_entry_safe(block, temp, &fs->block_freelist, list) {
        kfree(block);
    }
    kfree(fs);
}

static void ramfs_free_inode(struct inode *inode) {
    struct superblock *sb = i_sb(inode);
    struct ramfs *fs = sb_ram(sb);
    struct ramfs_inode *ri = i_ram(inode);
    struct ramfs_block *block, *temp;
    list_for_each_entry_safe(block, temp, &ri->block_list, list) {
        list_add(&block->list, &fs->block_freelist);
    }
    kfree(ri);
}

static struct ramfs_block *ramfs_find_block(struct ramfs_inode *ri,
                                            off_t cursor, off_t *offset) {
    // TODO: We can optimize this using backwards iteration if block index is
    // closer to file end
    size_t block_idx = cursor >> RAMFS_BLOCK_SIZE_BITS;
    if (block_idx >= ri->block_count) return NULL;

    struct ramfs_block *block =
        list_first_entry(&ri->block_list, struct ramfs_block, list);
    while (block_idx--) block = list_next_entry(block, list);
    *offset = cursor & (RAMFS_BLOCK_SIZE - 1);

    return block;
}

static struct ramfs_block *ramfs_append_block(struct ramfs *fs,
                                              struct ramfs_inode *ri) {
    struct ramfs_block *new_block = ramfs_get_empty_block(fs);
    if (new_block == NULL) return NULL;

    list_add_tail(&new_block->list, &ri->block_list);
    return new_block;
}

static struct ramfs_block *
ramfs_find_or_create_block(struct inode *inode, off_t cursor, off_t *offset) {
    struct ramfs_inode *ri = i_ram(inode);
    struct superblock *sb = i_sb(inode);
    struct ramfs *fs = sb_ram(sb);
    size_t block_idx = cursor >> RAMFS_BLOCK_SIZE_BITS;

    struct ramfs_block *block =
        list_first_entry(&ri->block_list, struct ramfs_block, list);
    for (; block_idx && block; --block_idx)
        block = list_next_entry_or_null(block, &ri->block_list, list);

    if (block_idx && !block) {
        while (block_idx) {
            block = ramfs_append_block(fs, ri);
            if (!block) return NULL;
        }
    }
    *offset = cursor & (RAMFS_BLOCK_SIZE - 1);

    return block;
}

static ssize_t ramfs_read(struct file *filp, void *buf, size_t count) {
    struct inode *inode = f_i(filp);
    struct ramfs_inode *ri = i_ram(inode);
    struct superblock *sb = i_sb(inode);
    off_t in_block_offset;
    struct ramfs_block *block =
        ramfs_find_block(ri, filp->offset, &in_block_offset);

    if (filp->offset + count > inode->size) count = inode->size - filp->offset;

    void *cursor = buf;
    while (count != 0 && block) {
        off_t to_read = __block_end(sb, filp->offset) - filp->offset;
        if (to_read > count) to_read = count;

        memcpy(cursor, block->data + in_block_offset, to_read);
        in_block_offset = 0;
        cursor += to_read;
        count -= to_read;
        block = list_next_entry_or_null(block, &ri->block_list, list);
    }

    return cursor - buf;
}

static ssize_t ramfs_write(struct file *filp, const void *buf, size_t count) {
    struct inode *inode = f_i(filp);
    struct ramfs_inode *ri = i_ram(inode);
    struct superblock *sb = i_sb(inode);
    struct ramfs *fs = sb_ram(sb);
    off_t in_block_offset;
    struct ramfs_block *block =
        ramfs_find_or_create_block(inode, filp->offset, &in_block_offset);
    if (!block) return -ENOMEM;

    const void *cursor = buf;
    while (count != 0 && block) {
        off_t to_write = __block_end(sb, filp->offset) - filp->offset;
        if (to_write > count) to_write = count;

        memcpy(block->data + in_block_offset, cursor, to_write);
        in_block_offset = 0;
        cursor += to_write;
        count -= to_write;
        block = list_next_entry_or_null(block, &ri->block_list, list);
        if (block == NULL) {
            block = ramfs_append_block(fs, ri);
            if (!block) return -ENOMEM;
        }
    }
    ssize_t total_wrote = cursor - buf;
    off_t result_pos = total_wrote + filp->offset;
    if (result_pos > inode->size) {
        inode->size = result_pos;
        ri->block_count = result_pos >> RAMFS_BLOCK_SIZE_BITS;
    }

    return total_wrote;
}

static const struct sb_ops sb_ops = {.release_sb = ramfs_release_sb};
static const struct inode_ops inode_ops = {.free = ramfs_free_inode};
static const struct file_ops file_ops = {.lseek = generic_lseek,
                                         .read = ramfs_read,
                                         .write = ramfs_write,
                                         .readdir = ramfs_readdir};

static struct inode *ramfs_create_inode(void) {
    struct inode *inode = alloc_inode();
    if (!inode) return NULL;

    struct ramfs_inode *ri = kzalloc(sizeof(*ri));
    if (!ri) {
        kfree(inode);
        return NULL;
    }
    init_list_head(&ri->block_list);
    inode->private = ri;
    inode->ops = &inode_ops;
    inode->file_ops = &file_ops;
    return inode;
}

static struct dentry *ramfs_create_root(void) {
    struct dentry *entry = create_root_dentry();
    if (!entry) return NULL;

    struct inode *inode = ramfs_create_inode();
    if (!inode) {
        kfree(entry);
        return NULL;
    }

    init_dentry(entry, inode);
    return entry;
}

int ramfs_mount(struct superblock *sb) {
    struct ramfs *fs = kzalloc(sizeof(*fs));
    if (!fs) return -ENOMEM;

    init_list_head(&fs->block_freelist);
    sb->private = fs;
    sb->blk_sz = RAMFS_BLOCK_SIZE;
    sb->blk_sz_bits = RAMFS_BLOCK_SIZE_BITS;
    sb->ops = &sb_ops;

    struct dentry *root = ramfs_create_root();
    if (!root) {
        kfree(fs);
        return -ENOMEM;
    }

    return 0;
}
