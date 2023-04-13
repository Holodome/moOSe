#include <assert.h>
#include <errno.h>
#include <fs/ramfs.h>
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
        struct {
            struct list_head block_list;
            struct inode *parent;
        };
        struct list_head dentry_list;
    };
};

struct ramfs_file {
    u32 dirent_index;
    int is_finished;
};

struct ramfs {
    ino_t inode_counter;
};

static int ramfs_setattr(struct inode *) {
    return -ENOTSUP;
}
static void ramfs_release_sb(struct superblock *sb);
static void ramfs_free_inode(struct inode *inode);
static int ramfs_truncate(struct inode *inode);
static ssize_t ramfs_read(struct file *filp, void *buf, size_t count);
static ssize_t ramfs_write(struct file *filp, const void *buf, size_t count);
static int ramfs_mkdir(struct inode *dir, struct dentry *entry, mode_t mode);
static int ramfs_rmdir(struct inode *dir, struct dentry *entry);
static int ramfs_create(struct inode *dir, struct dentry *entry, mode_t mode);
static int ramfs_lookup(struct inode *dir, struct dentry *entry);
static int ramfs_rename(struct inode *olddir, struct dentry *oldentry,
                        struct inode *newdir, struct dentry *newentry);
static int ramfs_readdir(struct file *filp, struct dentry *entry);
static int ramfs_unlink(struct inode *dir, struct dentry *entry);
static int ramfs_mknod(struct inode *dir, struct dentry *entry, mode_t mode,
                       dev_t dev);
static int ramfs_open_file(struct inode *inode, struct file *filp);
static void ramfs_release_file(struct file *filp);

static const struct sb_ops sb_ops = {.release_sb = ramfs_release_sb};
static const struct inode_ops dir_inode_ops = {.lookup = ramfs_lookup,
                                               .free = ramfs_free_inode,
                                               .mkdir = ramfs_mkdir,
                                               .rmdir = ramfs_rmdir,
                                               .create = ramfs_create,
                                               .rename = ramfs_rename,
                                               .setattr = ramfs_setattr,
                                               .unlink = ramfs_unlink

};
static const struct inode_ops file_inode_ops = {
    .free = ramfs_free_inode,
    .truncate = ramfs_truncate,
    .setattr = ramfs_setattr,
};

static const struct file_ops file_ops = {.lseek = generic_lseek,
                                         .read = ramfs_read,
                                         .write = ramfs_write,
                                         .release = ramfs_release_file,
                                         .open = ramfs_open_file,
                                         .readdir = ramfs_readdir};

static struct ramfs *sb_ram(const struct superblock *sb) {
    return sb->private;
}
static struct ramfs_inode *i_ram(const struct inode *inode) {
    return inode->private;
}
static struct ramfs_file *f_ram(const struct file *filp) {
    return filp->private;
}

static void ramfs_release_dentry(struct ramfs_dentry *entry) {
    list_remove(&entry->list);
    release_inode(entry->inode);
    kfree(entry);
}

static struct ramfs_dentry *ramfs_get_empty_dentry(void) {
    struct ramfs_dentry *dentry = kmalloc(sizeof(*dentry));
    if (dentry != NULL)
        init_list_head(&dentry->list);
    return dentry;
}

static struct inode *ramfs_create_inode(struct inode *dir, mode_t mode) {
    struct superblock *sb = dir->sb;
    struct ramfs *fs = sb_ram(sb);
    struct inode *inode = alloc_inode();
    if (!inode)
        return NULL;

    struct ramfs_inode *ri = kmalloc(sizeof(*ri));
    if (!ri) {
        kfree(inode);
        return NULL;
    }
    inode->mode = mode;
    if (S_ISDIR(mode)) {
        inode->ops = &dir_inode_ops;
        ri->parent = dir;
        init_list_head(&ri->dentry_list);
    } else {
        inode->ops = &file_inode_ops;
        init_list_head(&ri->block_list);
    }
    inode->private = ri;
    inode->file_ops = &file_ops;
    inode->ino = fs->inode_counter++;
    list_add(&inode->sb_list, &sb->inode_list);
    return inode;
}

static void ramfs_release_sb(struct superblock *sb) {
    struct ramfs *fs = sb_ram(sb);
    kfree(fs);
}

static void ramfs_free_inode(struct inode *inode) {
    struct ramfs_inode *ri = i_ram(inode);
    struct ramfs_block *block, *temp;
    list_for_each_entry_safe(block, temp, &ri->block_list, list) {
        kfree(block);
    }
    list_remove(&inode->sb_list);
    kfree(ri);
}

static struct ramfs_block *ramfs_find_block(struct inode *inode, off_t cursor,
                                            off_t *offset) {
    struct ramfs_inode *ri = i_ram(inode);
    // TODO: We can optimize this using backwards iteration if block index is
    // closer to file end
    size_t block_idx = cursor >> RAMFS_BLOCK_SIZE_BITS;
    if (block_idx >= inode->block_count)
        return NULL;

    struct ramfs_block *block =
        list_first_entry(&ri->block_list, struct ramfs_block, list);
    while (block_idx--)
        block = list_next_entry(block, list);
    *offset = cursor & (RAMFS_BLOCK_SIZE - 1);

    return block;
}

static struct ramfs_block *ramfs_append_block(struct inode *inode) {
    struct ramfs_inode *ri = i_ram(inode);
    struct ramfs_block *new_block = kmalloc(sizeof(*new_block));
    if (new_block == NULL)
        return NULL;

    list_add_tail(&new_block->list, &ri->block_list);
    return new_block;
}

static void ramfs_pop_block(struct inode *inode) {
    expects(S_ISREG(inode->mode));
    struct ramfs_inode *ri = i_ram(inode);
    struct ramfs_block *block =
        list_last_or_null(&ri->block_list, struct ramfs_block, list);
    assert(block);
    list_remove(&block->list);
    kfree(block);
}

static struct ramfs_block *
ramfs_find_or_create_block(struct inode *inode, off_t cursor, off_t *offset) {
    struct ramfs_inode *ri = i_ram(inode);
    size_t block_idx = cursor >> RAMFS_BLOCK_SIZE_BITS;

    struct ramfs_block *block =
        list_first_entry(&ri->block_list, struct ramfs_block, list);
    for (; block_idx && block; --block_idx)
        block = list_next_entry_or_null(block, &ri->block_list, list);

    if (block_idx && !block) {
        while (block_idx) {
            block = ramfs_append_block(inode);
            if (!block)
                return NULL;
        }
    }
    *offset = cursor & (RAMFS_BLOCK_SIZE - 1);

    return block;
}

static ssize_t ramfs_read(struct file *filp, void *buf, size_t count) {
    struct inode *inode = file_inode(filp);
    struct ramfs_inode *ri = i_ram(inode);
    struct superblock *sb = inode->sb;
    off_t in_block_offset = 0;
    struct ramfs_block *block =
        ramfs_find_block(inode, filp->offset, &in_block_offset);

    if (filp->offset + count > inode->size)
        count = inode->size - filp->offset;

    void *cursor = buf;
    while (count != 0 && block) {
        off_t to_read = __block_end(sb, filp->offset) - filp->offset;
        if (to_read > count)
            to_read = count;

        memcpy(cursor, block->data + in_block_offset, to_read);
        in_block_offset = 0;
        cursor += to_read;
        count -= to_read;
        block = list_next_entry_or_null(block, &ri->block_list, list);
    }

    return cursor - buf;
}

static ssize_t ramfs_write(struct file *filp, const void *buf, size_t count) {
    struct inode *inode = file_inode(filp);
    struct ramfs_inode *ri = i_ram(inode);
    struct superblock *sb = inode->sb;
    off_t in_block_offset;
    struct ramfs_block *block =
        ramfs_find_or_create_block(inode, filp->offset, &in_block_offset);
    if (!block)
        return -ENOMEM;

    const void *cursor = buf;
    while (count != 0 && block) {
        off_t to_write = __block_end(sb, filp->offset) - filp->offset;
        if (to_write > count)
            to_write = count;

        memcpy(block->data + in_block_offset, cursor, to_write);
        in_block_offset = 0;
        cursor += to_write;
        count -= to_write;
        block = list_next_entry_or_null(block, &ri->block_list, list);
        if (block == NULL) {
            block = ramfs_append_block(inode);
            if (!block)
                return -ENOMEM;
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
        while (count_to_delete)
            ramfs_pop_block(inode);
        inode->block_count = new_blocks;
    } else if (old_blocks < new_blocks) {
        off_t count_to_append = new_blocks - old_blocks;
        while (count_to_append) {
            struct ramfs_block *block = ramfs_append_block(inode);
            if (block == NULL)
                return -ENOMEM;
            memset(block->data, 0, sizeof(block->data));
        }
        inode->block_count = new_blocks;
    }

    return 0;
}

static struct ramfs_dentry *ramfs_find_by_name(struct inode *dir,
                                               const char *name) {
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

static int ramfs_lookup(struct inode *dir, struct dentry *entry) {
    if (strcmp(entry->name, ".") == 0) {
        init_dentry(entry, dir);
        return 0;
    }
    if (strcmp(entry->name, "..") == 0) {
        struct ramfs_inode *ri = i_ram(dir);
        struct inode *parent = ri->parent;
        if (parent == NULL)
            parent = dir;
        init_dentry(entry, dir);
        return 0;
    }
    struct ramfs_dentry *found = ramfs_find_by_name(dir, entry->name);
    if (found == NULL)
        return -ENOENT;

    init_dentry(entry, found->inode);
    return 0;
}

static int ramfs_mknod(struct inode *dir, struct dentry *entry, mode_t mode,
                       dev_t dev) {
    (void)dev;
    if (ramfs_find_by_name(dir, entry->name))
        return -EEXIST;

    struct inode *new_inode = ramfs_create_inode(dir, mode);
    struct ramfs_dentry *new_dentry = ramfs_get_empty_dentry();
    if (!new_dentry) {
        release_inode(new_inode);
        return -ENOMEM;
    }
    new_dentry->inode = new_inode;

    size_t test_name_len = strlen(entry->name);
    if (test_name_len > RAMFS_NAME_LEN) {
        ramfs_release_dentry(new_dentry);
        return -ENAMETOOLONG;
    }

    struct ramfs_inode *dir_ir = i_ram(dir);
    memcpy(new_dentry->name, entry->name, test_name_len);
    new_dentry->name_len = test_name_len;
    list_add(&new_dentry->list, &dir_ir->dentry_list);
    if (S_ISDIR(mode))
        i_ram(new_inode)->parent = dir;

    return 0;
}

static int ramfs_mkdir(struct inode *dir, struct dentry *entry, mode_t mode) {
    return ramfs_mknod(dir, entry, mode | S_IFDIR, 0);
}

static int ramfs_create(struct inode *dir, struct dentry *entry, mode_t mode) {
    return ramfs_mknod(dir, entry, mode | S_IFREG, 0);
}

static int ramfs_rename(struct inode *olddir, struct dentry *oldentry,
                        struct inode *newdir, struct dentry *newentry) {
    struct ramfs_dentry *rd = ramfs_find_by_name(olddir, oldentry->name);
    expects(rd && rd->inode == oldentry->inode);

    if (ramfs_find_by_name(newdir, newentry->name))
        return -EEXIST;

    newentry->inode = oldentry->inode;
    oldentry->inode = NULL;
    list_remove(&rd->list);
    list_add(&rd->list, &i_ram(newdir)->dentry_list);

    return 0;
}

static int ramfs_readdir(struct file *filp, struct dentry *entry) {
    struct ramfs_file *rf = f_ram(filp);
    if (rf->is_finished)
        return -ENOENT;

    struct inode *dir = filp->dentry->inode;
    struct ramfs_inode *ri = i_ram(dir);
    u32 index = rf->dirent_index;
    struct ramfs_dentry *iter =
        list_first_or_null(&ri->dentry_list, struct ramfs_dentry, list);
    while (index-- && iter)
        iter = list_next_entry_or_null(iter, &ri->dentry_list, list);
    if (iter == NULL)
        return -ENOENT;

    // TODO: iter->name is not zero-terminated!!!
    if (dentry_set_name(entry, iter->name))
        return -ENOMEM;
    init_dentry(entry, iter->inode);

    return 0;
}

static int ramfs_open_file(struct inode *, struct file *filp) {
    struct ramfs_file *rf = kzalloc(sizeof(*rf));
    if (!rf)
        return -ENOMEM;
    filp->private = rf;
    return 0;
}

static void ramfs_release_file(struct file *filp) {
    struct ramfs_file *file = f_ram(filp);
    kfree(file);
}

static int ramfs_unlink(struct inode *dir, struct dentry *entry) {
    struct inode *inode = entry->inode;
    struct ramfs_dentry *found = ramfs_find_by_name(dir, entry->name);
    expects(found && found->inode == inode);

    if (S_ISDIR(inode->mode)) {
        struct ramfs_inode *ri = i_ram(inode);
        if (!list_is_empty(&ri->dentry_list))
            return -ENOTEMPTY;
    }
    ramfs_release_dentry(found);

    return 0;
}

static int ramfs_rmdir(struct inode *dir, struct dentry *entry) {
    return ramfs_unlink(dir, entry);
}

static struct inode *ramfs_create_root_inode(struct superblock *sb) {
    struct ramfs *fs = sb_ram(sb);
    struct inode *inode = alloc_inode();
    if (!inode)
        return NULL;

    mode_t mode = 0666 | S_IFDIR;
    struct ramfs_inode *ri = kmalloc(sizeof(*ri));
    if (!ri) {
        kfree(inode);
        return NULL;
    }
    inode->mode = mode;
    inode->ops = &dir_inode_ops;
    ri->parent = NULL;
    init_list_head(&ri->dentry_list);
    inode->private = ri;
    inode->file_ops = &file_ops;
    inode->ino = fs->inode_counter++;
    list_add(&inode->sb_list, &sb->inode_list);
    return inode;
}

static struct dentry *ramfs_create_root(struct superblock *sb) {
    struct dentry *entry = create_root_dentry();
    if (!entry)
        return NULL;

    struct inode *inode = ramfs_create_root_inode(sb);
    if (!inode) {
        kfree(entry);
        return NULL;
    }

    init_dentry(entry, inode);
    return entry;
}

int ramfs_mount(struct superblock *sb) {
    struct ramfs *fs = kzalloc(sizeof(*fs));
    if (!fs)
        return -ENOMEM;

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
