#include <fs/ramfs.h>

#include <assert.h>
#include <mm/kmalloc.h>
#include <string.h>

#define RAMFS_BLOCK_SIZE 4096
#define RAMFS_BLOCK_SIZE_BITS 12
#define RAMFS_NAME_LEN 256

struct ramfs_block {
    struct list_head list;
    char data[RAMFS_BLOCK_SIZE];
};

struct ramfs_dentry {
    struct list_head list;
    struct inode *inode;
    u32 name_len;
    char name[RAMFS_NAME_LEN];
};

struct ramfs_inode {
    union {
        struct list_head list;
        struct list_head block_list;
        struct list_head dentry_list;
    };
};

struct ramfs {
    ino_t inode_counter;
    struct list_head block_freelist;
    struct list_head dentry_freelist;
    struct list_head inode_freelist;
};

static const struct sb_ops sb_ops = {.release_sb = ramfs_release_sb};
static const struct inode_ops inode_ops = {.free = ramfs_free_inode,
                                           .truncate = ramfs_truncate};
static const struct file_ops file_ops = {.lseek = generic_lseek,
                                         .read = ramfs_read,
                                         .write = ramfs_write,
                                         .readdir = ramfs_readdir};

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

static struct ramfs_dentry *ramfs_get_empty_dentry(struct ramfs *fs) {
    struct ramfs_dentry *dentry =
        list_first_or_null(&fs->dentry_freelist, struct ramfs_dentry, list);
    if (!dentry)
        dentry = kmalloc(sizeof(*dentry));
    else
        list_remove(&dentry->list);
    return dentry;
}

static struct ramfs_inode *ramfs_get_empty_inode(struct ramfs *fs) {
    struct ramfs_inode *inode =
        list_first_or_null(&fs->inode_freelist, struct ramfs_inode, list);
    if (!inode)
        inode = kmalloc(sizeof(*inode));
    else
        list_remove(&inode->list);
    return inode;
}

static struct inode *ramfs_create_inode(struct superblock *sb) {
    struct ramfs *fs = sb_ram(sb);
    struct inode *inode = alloc_inode();
    if (!inode) return NULL;

    struct ramfs_inode *ri = ramfs_get_empty_inode(fs);
    if (!ri) {
        kfree(inode);
        return NULL;
    }
    init_list_head(&ri->block_list);
    inode->private = ri;
    inode->ops = &inode_ops;
    inode->file_ops = &file_ops;
    list_add(&inode->sb_list, &sb->inode_list);
    return inode;
}

static void ramfs_release_sb(struct superblock *sb) {
    struct ramfs *fs = sb_ram(sb);
    {
        struct ramfs_block *it, *temp;
        list_for_each_entry_safe(it, temp, &fs->block_freelist, list) {
            kfree(it);
        }
    }
    {
        struct ramfs_inode *it, *temp;
        list_for_each_entry_safe(it, temp, &fs->inode_freelist, list) {
            kfree(it);
        }
    }
    {
        struct ramfs_dentry *it, *temp;
        list_for_each_entry_safe(it, temp, &fs->dentry_freelist, list) {
            kfree(it);
        }
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
    list_remove(&inode->sb_list);
    list_add(&ri->list, &fs->inode_freelist);
}

static struct ramfs_block *ramfs_find_block(struct inode *inode, off_t cursor,
                                            off_t *offset) {
    expects(S_ISREG(inode->mode));
    struct ramfs_inode *ri = i_ram(inode);
    // TODO: We can optimize this using backwards iteration if block index is
    // closer to file end
    size_t block_idx = cursor >> RAMFS_BLOCK_SIZE_BITS;
    if (block_idx >= inode->block_count) return NULL;

    struct ramfs_block *block =
        list_first_entry(&ri->block_list, struct ramfs_block, list);
    while (block_idx--) block = list_next_entry(block, list);
    *offset = cursor & (RAMFS_BLOCK_SIZE - 1);

    return block;
}

static struct ramfs_block *ramfs_append_block(struct inode *inode) {
    expects(S_ISREG(inode->mode));
    struct superblock *sb = i_sb(inode);
    struct ramfs *fs = sb_ram(sb);
    struct ramfs_inode *ri = i_ram(inode);
    struct ramfs_block *new_block = ramfs_get_empty_block(fs);
    if (new_block == NULL) return NULL;

    list_add_tail(&new_block->list, &ri->block_list);
    return new_block;
}

static void ramfs_pop_block(struct inode *inode) {
    expects(S_ISREG(inode->mode));
    struct superblock *sb = i_sb(inode);
    struct ramfs *fs = sb_ram(sb);
    struct ramfs_inode *ri = i_ram(inode);
    struct ramfs_block *block =
        list_last_or_null(&ri->block_list, struct ramfs_block, list);
    assert(block);
    list_remove(&block->list);
    list_add(&block->list, &fs->block_freelist);
}

static struct ramfs_block *
ramfs_find_or_create_block(struct inode *inode, off_t cursor, off_t *offset) {
    expects(S_ISREG(inode->mode));
    struct ramfs_inode *ri = i_ram(inode);
    size_t block_idx = cursor >> RAMFS_BLOCK_SIZE_BITS;

    struct ramfs_block *block =
        list_first_entry(&ri->block_list, struct ramfs_block, list);
    for (; block_idx && block; --block_idx)
        block = list_next_entry_or_null(block, &ri->block_list, list);

    if (block_idx && !block) {
        while (block_idx) {
            block = ramfs_append_block(inode);
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
        ramfs_find_block(inode, filp->offset, &in_block_offset);

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
            block = ramfs_append_block(inode);
            if (!block) return -ENOMEM;
        }
    }
    ssize_t total_wrote = cursor - buf;
    off_t result_pos = total_wrote + filp->offset;
    if (result_pos > inode->size) {
        inode->size = result_pos;
        inode->block_count = result_pos >> RAMFS_BLOCK_SIZE_BITS;
    }

    return total_wrote;
}

static int ramfs_truncate(struct inode *inode) {
    off_t old_blocks = inode->block_count;
    off_t new_blocks = inode->size >> RAMFS_BLOCK_SIZE_BITS;
    if (old_blocks > new_blocks) {
        off_t count_to_delete = old_blocks - new_blocks;
        while (count_to_delete) ramfs_pop_block(inode);
        inode->block_count = new_blocks;
    } else if (old_blocks < new_blocks) {
        off_t count_to_append = new_blocks - old_blocks;
        while (count_to_append) {
            struct ramfs_block *block = ramfs_append_block(inode);
            if (block == NULL) return -ENOMEM;
            memset(block->data, 0, sizeof(block->data));
        }
        inode->block_count = new_blocks;
    }

    return 0;
}

static struct ramfs_dentry *ramfs_find_by_name(struct inode *dir,
                                               const char *name) {
    expects(S_ISDIR(dir->mode));
    struct ramfs_inode *ri = i_ram(dir);
    struct ramfs_dentry *it;
    size_t test_name_len = strlen(name);
    list_for_each_entry(it, &ri->dentry_list, list) {
        if (test_name_len == it->name_len &&
            memcmp(name, it->name, test_name_len) == 0) {
            return it;
        }
    }

    return NULL;
}

int ramfs_lookup(struct inode *dir, struct dentry *entry) {
    struct ramfs_dentry *found = ramfs_find_by_name(dir, entry->name);
    if (found == NULL) return -ENOENT;

    init_dentry(entry, found->inode);
    return 0;
}

static int ramfs_chattr(struct inode *inode __unused) { return 0; }

static int ramfs_mkdir(struct inode *dir, struct dentry *entry) {
    expects(S_ISDIR(dir->mode));
    if (ramfs_find_by_name(dir, entry->name)) return -EEXIST;

    struct superblock *sb = i_sb(dir);
    struct ramfs *fs = sb_ram(sb);
    struct ramfs_dentry *rd = ramfs_get_empty_dentry(fs);
    if (rd == NULL) return -ENOMEM;

    size_t test_name_len = strlen(entry->name);
    if (test_name_len > RAMFS_NAME_LEN) return -ENAMETOOLONG;
    memcpy(rd->name, entry->name, test_name_len);
    rd->name_len = test_name_len;

    struct inode *new_inode = ramfs_create_inode(sb);
    if (new_inode == NULL) {
        list_add(&rd->list, &fs->dentry_freelist);
        return -ENOMEM;
    }
    new_inode->mode = 0666 | S_IFDIR;

    struct ramfs_inode *ri = i_ram(dir);
    list_add(&rd->list, &ri->dentry_list);
    rd->inode = new_inode;

    return 0;
}

static struct dentry *ramfs_create_root(struct superblock *sb) {
    struct dentry *entry = create_root_dentry();
    if (!entry) return NULL;

    struct inode *inode = ramfs_create_inode(sb);
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

    struct dentry *root = ramfs_create_root(sb);
    if (!root) {
        kfree(fs);
        return -ENOMEM;
    }

    return 0;
}
