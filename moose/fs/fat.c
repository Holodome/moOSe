#include <assert.h>
#include <blk_device.h>
#include <ctype.h>
#include <endian.h>
#include <fs/fat.h>
#include <string.h>
#include <kstdio.h>

#define PFATFS_ROOTDIR ((u32)1)
#define PFATFS_DIRENT_SIZE 32

#define PFATFS_DIRENT_ATTR_RO 0x01
#define PFATFS_DIRENT_ATTR_HIDDEN 0x02
#define PFATFS_DIRENT_ATTR_SYSTEM 0x04
#define PFATFS_DIRENT_ATTR_VOL_ID 0x08
#define PFATFS_DIRENT_ATTR_DIR 0x10
#define PFATFS_DIRENT_ATTR_ARCHIVE 0x20
#define PFATFS_DIRENT_ATTR_LONG_NAME                                           \
    (PFATFS_DIRENT_ATTR_RO | PFATFS_DIRENT_ATTR_HIDDEN |                       \
     PFATFS_DIRENT_ATTR_SYSTEM | PFATFS_DIRENT_ATTR_VOL_ID)

// FAT is a structure where each entry specifies allocation of cluster with
// corresponding index. FAT filesystems differ in size of FAT entry, with 12
// being minimum and 32 maximum. When loading each entry we adjust them to
// FAT32-style ones, so that FAT32 has no differences and others are able to fit
// in unified format.
#define PFATFS_FAT12_FREE 0x000
#define PFATFS_FAT12_BAD 0xff7
#define PFATFS_FAT12_EOF 0xff8

#define PFATFS_FAT16_FREE 0x0000
#define PFATFS_FAT16_BAD 0xfff7
#define PFATFS_FAT16_EOF 0xfff8

#define PFATFS_FAT32_FREE 0x00000000
#define PFATFS_FAT32_BAD 0x0ffffff7
#define PFATFS_FAT32_EOF 0x0ffffff8

#define PFATFS_FAT_FREE PFATFS_FAT32_FREE
#define PFATFS_FAT_BAD PFATFS_FAT32_BAD
#define PFATFS_FAT_EOF PFATFS_FAT32_EOF
#define PFATFS_IS_FAT_REGULAR(_fat)                                            \
    (((_fat) != PFATFS_FAT_FREE) && ((_fat) < PFATFS_FAT_BAD))

#define PFATFS_IS_LEGACY_ROOTDIR(_fs, _file)                                   \
    (((_fs)->kind == PFATFS_FAT12 || (_fs)->kind == PFATFS_FAT16) &&           \
     is_rootdir(_file))

struct dirent {
    char name[11];
    u8 attr;
    u8 crt_time_tenth;
    u16 crt_time;
    u16 crt_date;
    u16 lst_acc_date;
    u32 fst_clus;
    u16 wrt_time;
    u16 wrt_date;
    u32 file_size;
};

struct ldirent {
    u16 name[13];
    u8 ord;
    u8 attr;
    u8 type;
    u8 checksum;
};

struct fpath_iter {
    unsigned is_finished : 1;
    unsigned is_last : 1;
    int part_len;
    const char *part;
    const char *cursor;
    const char *end;
};

static size_t extract_basename(const char *filename, size_t filename_len) {
#if 1
    size_t last_slash = filename_len - 1;
    for (; last_slash != 0; --last_slash) {
        if (filename[last_slash] == '/') {
            break;
        }
    }
    return last_slash;
#else
    return strnchr(filename, '/', filename_len) - filename;
#endif
}

static int is_illegal_dirname(u8 c) {
    // this LUT is generated using following script:
    /*
    illegal = list(range(0x20));
    illegal.extend([0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D,
    0x3E, 0x3F, 0x5B, 0x5C, 0x5D, 0x7C]); for i in range(0, 256, 8): val = 0 for
    j in range(8): val |= int(i + j in illegal) << j; print("0x%02x" % val,
    end=", " if i != 248 else "") print()
    */
    static const u8 lut[32] = {0xff, 0xff, 0xff, 0xff, 0x04, 0xdc, 0x00, 0xfc,
                               0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x10,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    return (lut[c >> 3] & (c & 0x7)) != 0;
}

#if 0
static u8 checksum(u8 name[11]) {
    u8 sum;
    for (size_t i = 0; i < 11; ++i)
        sum = (((sum & 0x01) != 0) ? 0x80 : 0x00) + (sum >> 1) + *name++;
    return sum;
}
#endif

static u16 encode_date(struct fatfs_date date) {
    if (date.day < 1)
        date.day = 1;
    else if (date.day > 31)
        date.day = 31;

    if (date.month < 1)
        date.month = 1;
    else if (date.month > 12)
        date.month = 12;

    if (date.year > 127)
        date.year = 127;

    return date.day | (date.month << 5) | (date.year << 9);
}

#if 0
static fatfs_date decode_date(u16 val) {
    fatfs_date date;
    date.day = val & 0x1F;
    date.month = (val >> 5) & 0x3F;
    date.year = val >> 9;
    return date;
}
#endif

static u16 encode_time(struct fatfs_time time) {
    time.secs = time.secs >> 1;
    if (time.secs > 29)
        time.secs = 29;
    if (time.mins > 59)
        time.mins = 59;
    if (time.hours > 23)
        time.hours = 23;

    return time.secs | (time.mins << 5) | (time.hours << 11);
}

#if 0
static fatfs_time decode_time(u16 val) {
    fatfs_time time;
    time.secs = val & 0x0F;
    time.mins = (val >> 5) & 0x1F;
    time.hours = val >> 11;
    return time;
}
#endif

static int n83_eq_reg(const char *n83, const char *reg, size_t reg_length) {
    size_t cursor = 0;
    for (; cursor < 8 && cursor < reg_length; ++cursor) {
        int s = toupper(reg[cursor]);
        if (s == '.') {
            if (n83[cursor] != ' ')
                return 0;
            break;
        }
        if (s != n83[cursor])
            return 0;
    }

    if (cursor == reg_length)
        return 1;

    ++cursor;
    for (size_t i = 0; i < 3 && cursor < reg_length; ++i, ++cursor) {
        int s = toupper(reg[cursor]);

        if (s != n83[8 + i])
            return 0;
    }

    return cursor == reg_length;
}

static int is_rootdir(struct fatfs_file *file) {
    assert(file->dirent_loc != PFATFS_ROOTDIR || file->type == FATFS_FILE_DIR);
    return file->dirent_loc == PFATFS_ROOTDIR;
}

static u32 cluster_to_bytes(struct fatfs *fs, u32 cluster) {
    assert(cluster >= 2);
    return (cluster - 2) * fs->bytes_per_cluster + fs->data_offset;
}

// TODO: Properly test how this works
static void iter_fpath_advance(struct fpath_iter *iter) {
start:
    if (iter->cursor >= iter->end) {
        iter->is_finished = 1;
        return;
    }

    for (; iter->cursor < iter->end; ++iter->part_len) {
        int c = *iter->cursor++;
        if (c == '/')
            break;
    }

    if (iter->part_len == 0)
        goto start;

    if (iter->cursor == iter->end)
        iter->is_last = 1;
}

static struct fpath_iter iter_fpath(const char *filename, size_t len) {
    struct fpath_iter iter = {};
    iter.cursor = filename;
    iter.part = filename;
    iter.end = filename + len;
    iter_fpath_advance(&iter);
    return iter;
}

static void iter_fpath_next(struct fpath_iter *iter) {
    iter->part = iter->cursor;
    iter->part_len = 0;
    iter_fpath_advance(iter);
}

// Fat type is determined only by the total cluster count
static enum fatfs_kind decide_kind(u32 clusters) {
    enum fatfs_kind kind;
    if (clusters < 4085)
        kind = PFATFS_FAT12;
    else if (clusters < 65525)
        kind = PFATFS_FAT16;
    else
        kind = PFATFS_FAT32;

    return kind;
}

static int parse_bpb(struct fatfs *fs) {
    // skip the boot sector stuff
    // read the part of the boot sector that has data we are interested in
    u8 bytes[25];
    blk_read(fs->dev, 11, bytes, sizeof(bytes));

    u16 byts_per_sec;
    memcpy(&byts_per_sec, bytes, sizeof(byts_per_sec));
    u8 sec_per_clus = bytes[2];
    u16 rsvd_sec_cnt;
    memcpy(&rsvd_sec_cnt, bytes + 3, sizeof(rsvd_sec_cnt));
    u8 num_fats = bytes[5];
    u16 root_ent_cnt;
    memcpy(&root_ent_cnt, bytes + 6, sizeof(root_ent_cnt));
    u16 tot_sec16;
    memcpy(&tot_sec16, bytes + 8, sizeof(tot_sec16));
    u16 fatsz16;
    memcpy(&fatsz16, bytes + 11, sizeof(fatsz16));
    u32 tot_sec32;
    memcpy(&tot_sec32, bytes + 21, sizeof(tot_sec32));

    // prevent divide by zero
    if (byts_per_sec == 0)
        return -EINVAL;

    u32 bytes_per_cluster = byts_per_sec * sec_per_clus;
    u32 root_dir_sectors =
        ((root_ent_cnt * PFATFS_DIRENT_SIZE) + (byts_per_sec - 1)) /
        byts_per_sec;

    u32 fatsz;
    u32 tot_sec;
    u32 root_clus = 0;

    int is_fat32 = fatsz16 == 0;
    if (is_fat32) {
        u8 bytes[12];
        blk_read(fs->dev, 11 + 25, bytes, sizeof(bytes));

        u32 fatsz32;
        memcpy(&fatsz32, bytes, sizeof(fatsz32));
        memcpy(&root_clus, bytes + 8, sizeof(root_clus));

        fatsz = fatsz32;
        tot_sec = tot_sec32;
    } else {
        fatsz = fatsz16;
        tot_sec = tot_sec16 != 0 ? tot_sec16 : tot_sec32;
    }

    u32 first_data_sector =
        rsvd_sec_cnt + (num_fats * fatsz) + root_dir_sectors;
    u32 data_sec = tot_sec - first_data_sector;
    u32 cluster_count = data_sec / sec_per_clus;
    enum fatfs_kind kind = decide_kind(cluster_count);

    fs->kind = kind;
    fs->bytes_per_cluster = bytes_per_cluster;
    fs->data_offset = first_data_sector * byts_per_sec;
    fs->fat_offset = rsvd_sec_cnt * byts_per_sec;
    fs->cluster_count = cluster_count;
    if (kind == PFATFS_FAT32) {
        fs->rootdir.cluster = root_clus;
    } else {
        fs->rootdir.legacy.offset =
            (first_data_sector - root_dir_sectors) * byts_per_sec;
        fs->rootdir.legacy.size = root_dir_sectors * byts_per_sec;
    }

    return 0;
}

int fatfs_mount(struct fatfs *fs) {
    int result = parse_bpb(fs);
    if (result != 0)
        return result;

    return 0;
}

static u32 get_fat(struct fatfs *fs, u32 cluster) {
    u32 fat = PFATFS_FAT_BAD;
    u32 fat_off = fs->fat_offset;
    u8 bytes[sizeof(u32)];
    switch (fs->kind) {
    case PFATFS_FAT12: {
        fat_off += cluster + (cluster >> 1);
        blk_read(fs->dev, fat_off, bytes, sizeof(u16));

        u16 packed;
        memcpy(&packed, bytes, sizeof(packed));
        if ((cluster & 1) != 0)
            fat = packed >> 4;
        else
            fat = packed & 0x0FFF;

        if (fat == PFATFS_FAT12_FREE)
            fat = PFATFS_FAT_FREE;
        else if (fat == PFATFS_FAT12_BAD)
            fat = PFATFS_FAT_BAD;
        else if (fat >= PFATFS_FAT12_EOF)
            fat = PFATFS_FAT_EOF;
    } break;
    case PFATFS_FAT16:
        fat_off += cluster * sizeof(u16);
        blk_read(fs->dev, fat_off, bytes, sizeof(u16));

        memcpy(&fat, bytes, sizeof(u16));
        if (fat == PFATFS_FAT16_FREE)
            fat = PFATFS_FAT_FREE;
        else if (fat == PFATFS_FAT16_BAD)
            fat = PFATFS_FAT_BAD;
        else if (fat >= PFATFS_FAT16_EOF)
            fat = PFATFS_FAT_EOF;
        break;
    case PFATFS_FAT32:
        fat_off += cluster * sizeof(u32);
        blk_read(fs->dev, fat_off, bytes, sizeof(u32));

        memcpy(&fat, bytes, sizeof(u32));
        break;
    }

    return fat;
}

static void set_fat(struct fatfs *fs, u32 cluster, u32 fat) {
    u8 bytes[sizeof(u32)];
    u32 fat_off = fs->fat_offset;

    switch (fs->kind) {
    case PFATFS_FAT12: {
        fat_off += cluster + (cluster >> 1);
        blk_read(fs->dev, fat_off, bytes, sizeof(u16));

        if (fat == PFATFS_FAT_FREE)
            fat = PFATFS_FAT12_FREE;
        else if (fat == PFATFS_FAT_BAD)
            fat = PFATFS_FAT12_BAD;
        else if (fat >= PFATFS_FAT_EOF)
            fat = PFATFS_FAT12_EOF;

        u16 old_packed;
        memcpy(&old_packed, bytes, sizeof(old_packed));
        if ((cluster & 1) != 0)
            fat = (old_packed & 0x000F) | (fat << 4);
        else
            fat = (old_packed & 0xF000) | fat;
        memcpy(bytes, &fat, sizeof(u16));
        blk_write(fs->dev, fat_off, bytes, sizeof(u16));
    } break;
    case PFATFS_FAT16:
        fat_off += fs->fat_offset + cluster * sizeof(u16);

        if (fat == PFATFS_FAT_FREE)
            fat = PFATFS_FAT16_FREE;
        else if (fat == PFATFS_FAT_BAD)
            fat = PFATFS_FAT16_BAD;
        else if (fat >= PFATFS_FAT_EOF)
            fat = PFATFS_FAT16_EOF;
        memcpy(bytes, &fat, sizeof(u16));
        blk_write(fs->dev, fat_off, bytes, sizeof(u16));
        break;
    case PFATFS_FAT32:
        fat_off += cluster * sizeof(u32);
        memcpy(bytes, &fat, sizeof(u32));
        blk_write(fs->dev, fat_off, bytes, sizeof(u32));
        break;
    }
}

static ssize_t find_free_cluster(struct fatfs *fs, u32 start_cluster,
                                 u32 end_cluster) {
    for (size_t cluster = start_cluster; cluster < end_cluster; ++cluster) {
        u32 fat = get_fat(fs, cluster);
        if (fat == PFATFS_FAT_FREE)
            return cluster;
    }

    return -ENOSPC;
}

static ssize_t allocate_cluster(struct fatfs *fs) {
    ssize_t cluster = find_free_cluster(fs, 0, fs->cluster_count);
    if (cluster < 0)
        return cluster;
    set_fat(fs, cluster, PFATFS_FAT_EOF);

    return cluster;
}

static void read_dirent(struct fatfs *fs, u32 loc, struct dirent *dirent) {
    u8 bytes[PFATFS_DIRENT_SIZE];
    blk_read(fs->dev, loc, bytes, sizeof(bytes));

    memcpy(dirent->name, bytes, sizeof(dirent->name));
    dirent->attr = bytes[11];
    dirent->crt_time_tenth = bytes[13];
    dirent->crt_time = *(u16 *)(bytes + 14);
    dirent->crt_date = *(u16 *)(bytes + 16);
    dirent->lst_acc_date = *(u16 *)(bytes + 18);
    dirent->fst_clus =
        ((u32) * (u16 *)(bytes + 20) << 16) | *(u16 *)(bytes + 26);
    dirent->wrt_time = *(u16 *)(bytes + 22);
    dirent->wrt_date = *(u16 *)(bytes + 24);
    dirent->file_size = *(u16 *)(bytes + 28);
}

static void write_dirent(struct fatfs *fs, u32 loc,
                         const struct dirent *dirent) {
    u8 bytes[PFATFS_DIRENT_SIZE];
    memcpy(bytes, dirent->name, sizeof(dirent->name));
    bytes[11] = dirent->attr;
    bytes[12] = 0;
    bytes[13] = dirent->crt_time_tenth;
    *(u16 *)(bytes + 16) = dirent->crt_time;
    *(u16 *)(bytes + 18) = dirent->crt_date;
    *(u16 *)(bytes + 20) = dirent->fst_clus >> 16;
    *(u16 *)(bytes + 22) = dirent->wrt_time;
    *(u16 *)(bytes + 24) = dirent->wrt_date;
    *(u16 *)(bytes + 26) = dirent->fst_clus & 0xFFFF;
    *(u32 *)(bytes + 28) = dirent->file_size;

    blk_write(fs->dev, loc, bytes, sizeof(bytes));
}

static struct fatfs_file get_rootdir(struct fatfs *fs) {
    u32 rootdir_clus = fs->rootdir.cluster;
    struct fatfs_file file = {.type = FATFS_FILE_DIR,
                              .start_cluster = rootdir_clus,
                              .cluster = rootdir_clus,
                              .dirent_loc = PFATFS_ROOTDIR};
    return file;
}

static ssize_t find_dirent(struct fatfs *fs, u32 start, u32 end,
                           struct dirent *dirent) {
    u32 offset = start;
    assert(offset % PFATFS_DIRENT_SIZE == 0);
    for (; offset < end; offset += PFATFS_DIRENT_SIZE) {
        struct dirent temp;
        read_dirent(fs, offset, &temp);

        int name0 = temp.name[0];
        if (name0 == 0xe5)
            continue;
        if (name0 == 0x00)
            return -ENOENT;

        *dirent = temp;
        break;
    }

    return offset;
}

static int iter_dir_next(struct fatfs *fs, struct fatfs_file *dir,
                         struct dirent *dirent) {
    if (PFATFS_IS_LEGACY_ROOTDIR(fs, dir)) {
        u32 rootdir_offset = fs->rootdir.legacy.offset;
        u32 rootdir_size = fs->rootdir.legacy.size;
        u32 offset = dir->offset;

        ssize_t new_offset = find_dirent(fs, rootdir_offset + offset,
                                         rootdir_offset + rootdir_size, dirent);
        if (new_offset < 0)
            return new_offset;

        offset = (u32)new_offset;
        assert(offset <= rootdir_offset + rootdir_size);
        if (offset == rootdir_offset + rootdir_size)
            return -ENOENT;

        dir->offset = offset + PFATFS_DIRENT_SIZE;
        return offset;
    } else {
        u32 cluster = dir->cluster;
        u16 cluster_offset = dir->cluster_offset;
        ssize_t new_offset;

        assert(cluster_offset % PFATFS_DIRENT_SIZE == 0);
    retry:
        new_offset = find_dirent(
            fs, cluster_to_bytes(fs, cluster) + cluster_offset,
            cluster_to_bytes(fs, cluster) + fs->bytes_per_cluster, dirent);
        if (new_offset < 0)
            return new_offset;
        cluster_offset = new_offset;

        if (cluster_offset == fs->bytes_per_cluster) {
            ssize_t new_cluster = get_fat(fs, cluster);
            if (new_cluster < 0)
                return new_cluster;

            if (!PFATFS_IS_FAT_REGULAR(new_cluster))
                return -EINVAL;

            cluster = new_cluster;
            cluster_offset = 0;
            goto retry;
        }

        dir->cluster = cluster;
        dir->cluster_offset = cluster_offset + PFATFS_DIRENT_SIZE;
        return cluster_to_bytes(fs, cluster) + cluster_offset;
    }

    return 0;
}

static ssize_t find_empty_dirent_(struct fatfs *fs, u32 start, u32 end) {
    u32 offset = start;
    assert(offset % PFATFS_DIRENT_SIZE == 0);
    for (; offset < end; offset += PFATFS_DIRENT_SIZE) {
        struct dirent temp;
        read_dirent(fs, offset, &temp);
        int name0 = temp.name[0];
        if (name0 == 0xe5 || name0 == 0x00)
            return offset;
    }

    return -ENOENT;
}

static ssize_t find_empty_dirent(struct fatfs *fs, struct fatfs_file *dir) {
    if (PFATFS_IS_LEGACY_ROOTDIR(fs, dir)) {
        u32 rootdir_offset = fs->rootdir.legacy.offset;
        u32 rootdir_size = fs->rootdir.legacy.size;
        u32 offset = dir->offset;

        ssize_t new_offset = find_empty_dirent_(fs, rootdir_offset + offset,
                                                rootdir_offset + rootdir_size);
        if (new_offset < 0)
            return new_offset;

        offset = (u32)new_offset;
        assert(offset <= rootdir_offset + rootdir_size);
        if (offset == rootdir_offset + rootdir_size)
            return -ENOENT;

        dir->offset = offset + PFATFS_DIRENT_SIZE;
        return offset;
    } else {
        u32 cluster = dir->cluster;
        u16 cluster_offset = dir->cluster_offset;
        ssize_t new_offset;
        assert(cluster_offset % PFATFS_DIRENT_SIZE == 0);
    retry:
        new_offset = find_empty_dirent_(
            fs, cluster_to_bytes(fs, cluster) + cluster_offset,
            cluster_to_bytes(fs, cluster) + fs->bytes_per_cluster);
        if (new_offset < 0)
            return new_offset;
        cluster_offset = new_offset;

        if (cluster_offset == fs->bytes_per_cluster) {
            ssize_t new_cluster = get_fat(fs, cluster);
            if (new_cluster < 0)
                return new_cluster;

            if (!PFATFS_IS_FAT_REGULAR(new_cluster))
                return -EINVAL;

            cluster = new_cluster;
            cluster_offset = 0;
            goto retry;
        }

        dir->cluster = cluster;
        dir->cluster_offset = cluster_offset + PFATFS_DIRENT_SIZE;
        return cluster_to_bytes(fs, cluster) + cluster_offset;
    }
}

static ssize_t add_dirent(struct fatfs *fs, struct fatfs_file *dir,
                          const struct dirent *dirent) {
    ssize_t dirent_loc = find_empty_dirent(fs, dir);
    if (dirent_loc < 0)
        return dirent_loc;

    write_dirent(fs, dirent_loc, dirent);
    return dirent_loc;
}

static void init_child(struct dirent *dirent, struct fatfs_file *child,
                       u32 dirent_loc) {
    assert(dirent_loc % PFATFS_DIRENT_SIZE == 0);
    memset(child, 0, sizeof(*child));
    memcpy(child->name, dirent->name, sizeof(dirent->name));
    child->type = (dirent->attr & PFATFS_DIRENT_ATTR_DIR) != 0 ? FATFS_FILE_DIR
                                                               : FATFS_FILE_REG;
    child->dirent_loc = dirent_loc;
    child->size = dirent->file_size;
    child->cluster = child->start_cluster = dirent->fst_clus;
}

static ssize_t dir_empty(struct fatfs *fs, struct fatfs_file *dir) {
    assert(dir->type == FATFS_FILE_DIR);

    for (;;) {
        struct dirent dirent;
        ssize_t result = iter_dir_next(fs, dir, &dirent);
        if (result == -ENOENT)
            break;
        if (result < 0)
            return result;

        const char *name = dirent.name;
        static const u8 dots[] = "..          ";
        if (memcmp(name, dots, 11) != 0 && memcmp(name, dots + 1, 11) != 0)
            return -ENOTEMPTY;
    }

    return 0;
}

static int find_child(struct fatfs *fs, struct fatfs_file *parent,
                      struct fatfs_file *child, const char *name,
                      size_t name_len) {
    assert(parent->type == FATFS_FILE_DIR);

    for (;;) {
        struct dirent dirent;
        ssize_t dirent_loc = iter_dir_next(fs, parent, &dirent);
        if (dirent_loc < 0)
            return dirent_loc;

        if (n83_eq_reg(dirent.name, name, name_len)) {
            init_child(&dirent, child, dirent_loc);
            return 0;
        }
    }

    return -ENOENT;
}

int fatfs_readdir(struct fatfs *fs, struct fatfs_file *dir,
                  struct fatfs_file *child) {
    if (dir->type != FATFS_FILE_DIR)
        return -ENOTDIR;

    struct dirent dirent;
    ssize_t dirent_loc;
    for (;;) {
        dirent_loc = iter_dir_next(fs, dir, &dirent);
        if (dirent_loc < 0)
            return dirent_loc;
        if (dirent.name[0] != 0x00 && (u8)dirent.name[0] != 0xe5)
            break;
    }

    init_child(&dirent, child, dirent_loc);
    return 0;
}

ssize_t fatfs_read(struct fatfs *fs, struct fatfs_file *file, void *buffer,
                   size_t count) {
    if (file->type != FATFS_FILE_REG)
        return -EISDIR;

    char *cursor = (char *)buffer;
    while (count != 0) {
        if (file->cluster_offset >= fs->bytes_per_cluster) {
            ssize_t new_cluster = get_fat(fs, file->cluster);
            if (new_cluster < 0)
                return new_cluster;

            if (new_cluster == PFATFS_FAT_EOF)
                goto end;
            if (!PFATFS_IS_FAT_REGULAR(new_cluster))
                return -EINVAL;

            file->cluster = new_cluster;
            file->cluster_offset = 0;
        }

        size_t to_read = count;
        if (file->cluster_offset + to_read > fs->bytes_per_cluster)
            to_read = fs->bytes_per_cluster - file->cluster_offset;
        if (file->offset + to_read > file->size)
            to_read = file->size - file->offset;

        blk_read(fs->dev,
                 cluster_to_bytes(fs, file->cluster) + file->cluster_offset,
                 cursor, to_read);

        cursor += to_read;
        count -= to_read;

        file->offset += to_read;
        if (file->offset == file->size)
            goto end;

        file->cluster_offset += to_read;
        assert(file->cluster_offset <= fs->bytes_per_cluster);
    }

end:
    return cursor - (char *)buffer;
}

ssize_t fatfs_write(struct fatfs *fs, struct fatfs_file *file,
                    const void *buffer, size_t count) {
    if (file->type != FATFS_FILE_REG)
        return -EISDIR;

    const char *cursor = (const char *)buffer;
    while (count != 0) {
        if (file->cluster_offset >= fs->bytes_per_cluster) {
            ssize_t next_cluster = get_fat(fs, file->cluster);
            if (next_cluster < 0)
                return next_cluster;

            if (next_cluster == PFATFS_FAT_EOF) {
                next_cluster = allocate_cluster(fs);
                if (next_cluster < 0)
                    return next_cluster;
                set_fat(fs, file->cluster, next_cluster);
            } else if (!PFATFS_IS_FAT_REGULAR(next_cluster)) {
                return -EINVAL;
            }

            file->cluster = next_cluster;
            file->cluster_offset = 0;
        }

        size_t to_write = count;
        if (file->cluster_offset + to_write > fs->bytes_per_cluster)
            to_write = fs->bytes_per_cluster - file->cluster_offset;

        blk_write(fs->dev,
                  cluster_to_bytes(fs, file->cluster) + file->cluster_offset,
                  cursor, to_write);

        cursor += to_write;
        count -= to_write;

        file->offset += to_write;
        file->cluster_offset += to_write;
        assert(file->cluster_offset <= fs->bytes_per_cluster);
    }

    struct dirent dirent;
    read_dirent(fs, file->dirent_loc, &dirent);

    if (dirent.file_size < file->offset) {
        dirent.file_size = file->offset;
        write_dirent(fs, file->dirent_loc, &dirent);
    }
    kprintf("here %u %u\n", file->size, file->offset);

    return cursor - (const char *)buffer;
}

int fatfs_truncate(struct fatfs *fs, struct fatfs_file *file, size_t length) {
    if (file->type != FATFS_FILE_REG)
        return -EISDIR;

    // find cluster specified by 'length'
    int result = fatfs_seek(fs, file, length, SEEK_SET);
    if (result < 0)
        return result;
    // iterate linker list of clusters marking them all 'empty'
    u32 cluster = get_fat(fs, file->cluster);
    set_fat(fs, file->cluster, PFATFS_FAT_EOF);

    if (result < 0)
        return result;
    if (PFATFS_IS_FAT_REGULAR(cluster)) {
        for (;;) {
            u32 next_cluster = get_fat(fs, cluster);

            if (next_cluster == PFATFS_FAT_EOF)
                break;
            if (!PFATFS_IS_FAT_REGULAR(next_cluster))
                return -EINVAL;

            cluster = next_cluster;
            set_fat(fs, cluster, PFATFS_FAT_FREE);
        }
    }

    struct dirent dirent;
    read_dirent(fs, file->dirent_loc, &dirent);
    dirent.file_size = length;
    write_dirent(fs, file->dirent_loc, &dirent);

    return 0;
}

int fatfs_seek(struct fatfs *fs, struct fatfs_file *file, off_t offset,
               int whence) {
    u32 new_offset;
    switch (whence) {
    case SEEK_CUR:
        new_offset = file->offset + offset;
        break;
    case SEEK_SET:
        new_offset = offset;
        break;
    case SEEK_END:
        new_offset = file->size + offset;
        break;
    default:
        return -EINVAL;
    }

    if (new_offset > file->size)
        return -EINVAL;

    if (new_offset == file->offset)
        return 0;

    if (new_offset < file->offset) {
        file->offset = 0;
        file->cluster_offset = 0;
        file->cluster = file->start_cluster;
    }

    assert(new_offset >= file->offset);
    while (file->offset != new_offset) {
        if (file->cluster_offset >= fs->bytes_per_cluster) {
            u32 new_cluster = get_fat(fs, file->cluster);
            if (!PFATFS_IS_FAT_REGULAR(new_cluster))
                return -EINVAL;

            file->cluster = new_cluster;
            file->cluster_offset = 0;
        }

        u32 to_advance = new_offset - file->offset;
        if (file->cluster_offset + to_advance > fs->bytes_per_cluster) {
            to_advance = fs->bytes_per_cluster - file->cluster_offset;
        }

        file->offset += to_advance;
        file->cluster_offset += to_advance;
    }

    return 0;
}

static int fatfs_openv(struct fatfs *fs, const char *filename,
                       size_t filename_len, struct fatfs_file *file) {
    struct fatfs_file search = get_rootdir(fs);
    for (struct fpath_iter iter = iter_fpath(filename, filename_len);
         !iter.is_finished; iter_fpath_next(&iter)) {

        struct fatfs_file child = {0};
        int result = find_child(fs, &search, &child, iter.part, iter.part_len);
        if (result != 0)
            return result;

        search = child;
        if (iter.is_last)
            break;
    }

    *file = search;

    return 0;
}

static int init_basename(struct dirent *dirent, const char *filename,
                         size_t filename_len) {
    memset(dirent->name, ' ', sizeof(dirent->name));
    const char *before_dot = filename;
    for (size_t cursor = 0;
         before_dot < filename + filename_len && *before_dot != '.';
         ++cursor, ++before_dot) {
        u8 c = toupper(*before_dot);
        if (is_illegal_dirname(c))
            return -EINVAL;
        dirent->name[cursor] = c;
    }

    const char *after_dot = before_dot + 1;
    for (size_t cursor = 8; after_dot < filename + filename_len && cursor < 11;
         ++cursor, ++after_dot) {
        u8 c = toupper(*after_dot);
        if (is_illegal_dirname(c))
            return -EINVAL;
        dirent->name[cursor] = c;
    }

    if (after_dot < filename + filename_len)
        return -EINVAL;

    return 0;
}

static int dirent_create(const char *filename, size_t filename_len,
                         const struct fatfs_file_create_info *info,
                         struct dirent *dirent) {
    int result = init_basename(dirent, filename, filename_len);
    if (result >= 0 && info != NULL) {
        dirent->attr = info->attrs;
        dirent->crt_time_tenth = (info->time.secs & 0x1) * 10;
        dirent->crt_time = encode_time(info->time);
        dirent->crt_date = dirent->lst_acc_date = dirent->wrt_date =
            encode_date(info->date);
    }

    return result;
}

static int fatfs_createv(struct fatfs *fs, const char *filename,
                         size_t filename_len,
                         const struct fatfs_file_create_info *info,
                         struct fatfs_file *file) {
    size_t basename_idx = extract_basename(filename, filename_len);
    struct fatfs_file dir;
    int result = fatfs_openv(fs, filename, basename_idx, &dir);
    if (result < 0)
        return result;
    assert(dir.type == FATFS_FILE_DIR);

    ssize_t first_cluster = allocate_cluster(fs);
    if (first_cluster < 0)
        return first_cluster;

    if (basename_idx != 0)
        ++basename_idx;

    struct dirent dirent = {0};
    result = dirent_create(filename + basename_idx, filename_len - basename_idx,
                           info, &dirent);
    if (result < 0)
        return result;
    dirent.fst_clus = first_cluster;
    ssize_t dirent_loc = add_dirent(fs, &dir, &dirent);
    if (dirent_loc < 0)
        return dirent_loc;

    init_child(&dirent, file, dirent_loc);
    return 0;
}

static int fatfs_renamev(struct fatfs *fs, const char *oldpath,
                         size_t oldpath_len, const char *newpath,
                         size_t newpath_length) {
    size_t new_basename_idx = extract_basename(newpath, newpath_length);
    struct fatfs_file new_dir;
    int result = fatfs_openv(fs, newpath, new_basename_idx, &new_dir);
    if (result < 0)
        return result;
    assert(new_dir.type == FATFS_FILE_DIR);

    if (new_basename_idx != 0)
        ++new_basename_idx;

    struct fatfs_file old;
    result = fatfs_openv(fs, oldpath, oldpath_len, &old);
    if (result < 0)
        return result;

    struct dirent dirent;
    read_dirent(fs, old.dirent_loc, &dirent);

    init_basename(&dirent, newpath, newpath_length);
    if (result < 0)
        return result;

    ssize_t new_dirent_loc = add_dirent(fs, &new_dir, &dirent);
    if (new_dirent_loc < 0)
        return new_dirent_loc;

    dirent.name[0] = 0xe5;
    write_dirent(fs, old.dirent_loc, &dirent);
    return 0;
}

static int fatfs_mkdirv(struct fatfs *fs, const char *filename,
                        size_t filename_len) {
    struct fatfs_file_create_info info = {0};
    info.attrs = PFATFS_DIRENT_ATTR_DIR;
    struct fatfs_file file;
    return fatfs_createv(fs, filename, filename_len, &info, &file);
}

static int fatfs_removev(struct fatfs *fs, const char *filename,
                         size_t filename_len) {
    struct fatfs_file search;
    int result = fatfs_openv(fs, filename, filename_len, &search);
    if (result < 0)
        return result;
    if (search.type == FATFS_FILE_DIR) {
        if (is_rootdir(&search))
            return -EINVAL;
        result = dir_empty(fs, &search);
        if (result < 0)
            return result;
    }

    struct dirent dirent;
    read_dirent(fs, search.dirent_loc, &dirent);
    dirent.name[0] = 0xe5;
    write_dirent(fs, search.dirent_loc, &dirent);
    return 0;
}

int fatfs_open(struct fatfs *fs, const char *filename,
               struct fatfs_file *file) {
    return fatfs_openv(fs, filename, strlen(filename), file);
}

int fatfs_rename(struct fatfs *fs, const char *oldpath, const char *newpath) {
    return fatfs_renamev(fs, oldpath, strlen(oldpath), newpath,
                         strlen(newpath));
}

int fatfs_mkdir(struct fatfs *fs, const char *path) {
    return fatfs_mkdirv(fs, path, strlen(path));
}

int fatfs_remove(struct fatfs *fs, const char *path) {
    return fatfs_removev(fs, path, strlen(path));
}

int fatfs_create(struct fatfs *fs, const char *filename,
                 const struct fatfs_file_create_info *info,
                 struct fatfs_file *file) {
    return fatfs_createv(fs, filename, strlen(filename), info, file);
}
