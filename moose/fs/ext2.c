#include <fs/ext2.h>

#include <device.h>
#include <errno.h>
#include <kstdio.h>
#include <mm/kmalloc.h>

#define EXT2_SB_OFFSET 1024
#define EXT2_BGD_OFFSET 2048

int ext2_mount(struct ext2_fs *fs) {
    if (lseek(fs->dev, EXT2_SB_OFFSET, SEEK_SET) < 0) return -EIO;
    if (read(fs->dev, &fs->sb, sizeof(fs->sb)) != sizeof(fs->sb)) return -EIO;

    if (lseek(fs->dev, EXT2_BGD_OFFSET, SEEK_SET) < 0) return -EIO;

    // TODO: Actual count should depend on filessytem partition size
    // roughly partition_size_in_blocks / (8 * block_size)
    size_t bgds_count = 1;
    size_t bgds_size = bgds_count * sizeof(struct ext2_group_desc);
    struct ext2_group_desc *bgds = kmalloc(bgds_size);
    for (size_t i = 0; i < bgds_count; ++i) {
        if (read(fs->dev, bgds + i, sizeof(*bgds)) != sizeof(*bgds))
            goto free_bgds;
    }

    fs->bgds_count = bgds_count;
    fs->bgds = bgds;

free_bgds:
    kfree(bgds);

    return 0;
}
