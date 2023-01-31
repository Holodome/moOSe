#include <fs/fat.h>
#include <kmem.h>

// TODO: Assert
#define PFATFS_ASSERT(...) (void)(__VA_ARGS__)

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
     pfatfs__is_rootdir(_file))

#define PFATFS_TRY(_call)                                                      \
    do {                                                                       \
        int _result = _call;                                                   \
        if (_result < 0)                                                       \
            return _result;                                                    \
    } while (0)

typedef struct {
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
} pfatfs__dirent;

typedef struct {
    u16 name[13];
    u8 ord;
    u8 attr;
    u8 type;
    u8 checksum;
} pfatfs__ldirent;

typedef struct {
    unsigned is_finished : 1;
    unsigned is_last : 1;
    int part_len;
    const char *part;
    const char *cursor;
    const char *end;
} pfatfs__fpath_iter;

static int pfatfs__toupper(int a) {
    if ('a' <= a && a <= 'z')
        a -= 'a' - 'A';
    return a;
}

static size_t pfatfs__extract_basename(const char *filename,
                                       size_t filename_len) {
    size_t last_slash = filename_len - 1;
    for (; last_slash != 0; --last_slash) {
        if (filename[last_slash] == '/') {
            --last_slash;
            break;
        }
    }
    return last_slash;
}

static u16 pfatfs__read16(const u8 *src) {
    return ((u16)src[1] << 8) | (u16)src[0];
}

static u32 pfatfs__read32(const u8 *src) {
    return ((u32)src[3] << 24) | ((u32)src[2] << 16) | ((u32)src[1] << 8) |
           (u32)src[0];
}

static u8 *pfatfs__write16(u8 *dst, u16 data) {
    u8 *dstc = dst;
    *dst++ = data & 0xff;
    *dst = data >> 8;
    return dstc;
}

static u8 *pfatfs__write32(u8 *dst, u32 data) {
    u8 *dstc = dst;
    *dst++ = data & 0xff;
    *dst++ = (data >> 8) & 0xff;
    *dst++ = (data >> 16) & 0xff;
    *dst = data >> 24;
    return dstc;
}

static int pfatfs__is_illegal_dirname(u8 c) {
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
static u8 pfatfs__checksum(u8 name[11]) {
    u8 sum;
    for (size_t i = 0; i < 11; ++i)
        sum = (((sum & 0x01) != 0) ? 0x80 : 0x00) + (sum >> 1) + *name++;
    return sum;
}
#endif 

static u16 pfatfs__encode_date(pfatfs_date date) {
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
static pfatfs_date pfatfs__decode_date(u16 val) {
    pfatfs_date date;
    date.day = val & 0x1F;
    date.month = (val >> 5) & 0x3F;
    date.year = val >> 9;
    return date;
}
#endif 

static u16 pfatfs__encode_time(pfatfs_time time) {
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
static pfatfs_time pfatfs__decode_time(u16 val) {
    pfatfs_time time;
    time.secs = val & 0x0F;
    time.mins = (val >> 5) & 0x1F;
    time.hours = val >> 11;
    return time;
}
#endif 

static int pfatfs__83_eq_reg(const char *n83, const char *reg,
                             size_t reg_length) {
    size_t cursor = 0;
    for (; cursor < 8 && cursor < reg_length; ++cursor) {
        int s = pfatfs__toupper(reg[cursor]);
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
        int s = pfatfs__toupper(reg[cursor]);

        if (s != n83[8 + i])
            return 0;
    }

    return cursor == reg_length;
}

static int pfatfs__is_rootdir(pfatfs_file *file) {
    PFATFS_ASSERT(file->dirent_loc != PFATFS_ROOTDIR ||
                  file->type == PFATFS_FILE_DIR);
    return file->dirent_loc == PFATFS_ROOTDIR;
}

static int pfatfs__read(pfatfs *fs, void *buf, size_t size) {
    ssize_t result = fs->settings->read(fs->settings->handle, buf, size);
    if (result < 0 || (size_t)result != size)
        return PFATFS_EIO;

    return 0;
}

static int pfatfs__write(pfatfs *fs, const void *buf, size_t size) {
    ssize_t result = fs->settings->write(fs->settings->handle, buf, size);
    if (result < 0 || (size_t)result != size)
        return PFATFS_EIO;

    return 0;
}

static int pfatfs__seek(pfatfs *fs, i32 off, pfatfs_whence whence) {
    ssize_t result = fs->settings->seek(fs->settings->handle, off, whence);
    if (result != 0)
        return PFATFS_EIO;

    return 0;
}

static u32 pfatfs__cluster_to_bytes(pfatfs *fs, u32 cluster, u16 offset) {
    PFATFS_ASSERT(cluster >= 2 && offset < fs->bytes_per_cluster);
    return (cluster - 2) * fs->bytes_per_cluster + offset + fs->data_offset;
}

static int pfatfs__seek_read(pfatfs *fs, i32 off, void *buf, size_t size) {
    int result = pfatfs__seek(fs, off, PFATFS_SEEK_SET);
    if (result == 0)
        result = pfatfs__read(fs, buf, size);
    return result;
}

static int pfatfs__seek_write(pfatfs *fs, i32 off, const void *buf,
                              size_t size) {
    int result = pfatfs__seek(fs, off, PFATFS_SEEK_SET);
    if (result == 0)
        result = pfatfs__write(fs, buf, size);
    return result;
}

// TODO: Properly test how this works
static void pfatfs__iter_fpath_advance(pfatfs__fpath_iter *iter) {
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

static pfatfs__fpath_iter pfatfs__iter_fpath(const char *filename, size_t len) {
    pfatfs__fpath_iter iter = {};
    iter.cursor = filename;
    iter.part = filename;
    iter.end = filename + len;
    pfatfs__iter_fpath_advance(&iter);
    return iter;
}

static void pfatfs__iter_fpath_next(pfatfs__fpath_iter *iter) {
    iter->part = iter->cursor;
    iter->part_len = 0;
    pfatfs__iter_fpath_advance(iter);
}

// Fat type is determined only by the total cluster count
static pfatfs_kind pfatfs__decide_kind(u32 clusters) {
    pfatfs_kind kind;
    if (clusters < 4085)
        kind = PFATFS_FAT12;
    else if (clusters < 65525)
        kind = PFATFS_FAT16;
    else
        kind = PFATFS_FAT32;

    return kind;
}

static int pfatfs__parse_bpb(pfatfs *fs) {
    // skip the boot sector stuff
    PFATFS_TRY(pfatfs__seek(fs, 11, PFATFS_SEEK_SET));

    // read the part of the boot sector that has data we are interested in
    u8 bytes[25];
    PFATFS_TRY(pfatfs__read(fs, bytes, sizeof(bytes)));

    u16 byts_per_sec = pfatfs__read16(bytes);
    u8 sec_per_clus = bytes[2];
    u16 rsvd_sec_cnt = pfatfs__read16(bytes + 3);
    u8 num_fats = bytes[5];
    u16 root_ent_cnt = pfatfs__read16(bytes + 6);
    u16 tot_sec16 = pfatfs__read16(bytes + 8);
    u16 fatsz16 = pfatfs__read16(bytes + 11);
    u32 tot_sec32 = pfatfs__read32(bytes + 21);

    // prevent divide by zero
    if (byts_per_sec == 0) {
        return PFATFS_ECORRUPTED;
    }

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
        PFATFS_TRY(pfatfs__read(fs, bytes, sizeof(bytes)));

        u32 fatsz32 = pfatfs__read32(bytes);
        root_clus = pfatfs__read32(bytes + 8);

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
    pfatfs_kind kind = pfatfs__decide_kind(cluster_count);

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

static int pfatfs__read_dirent_(pfatfs *fs, pfatfs__dirent *dirent) {
    u8 bytes[PFATFS_DIRENT_SIZE];
    PFATFS_TRY(pfatfs__read(fs, bytes, sizeof(bytes)));

    memcpy(dirent->name, bytes, sizeof(dirent->name));
    dirent->attr = bytes[11];
    dirent->crt_time_tenth = bytes[13];
    dirent->crt_time = pfatfs__read16(bytes + 14);
    dirent->crt_date = pfatfs__read16(bytes + 16);
    dirent->lst_acc_date = pfatfs__read16(bytes + 18);
    dirent->fst_clus =
        ((u32)pfatfs__read16(bytes + 20) << 16) | pfatfs__read16(bytes + 26);
    dirent->wrt_time = pfatfs__read16(bytes + 22);
    dirent->wrt_date = pfatfs__read16(bytes + 24);
    dirent->file_size = pfatfs__read32(bytes + 28);

    return 0;
}

static int pfatfs__write_dirent_(pfatfs *fs, const pfatfs__dirent *dirent) {
    u8 bytes[PFATFS_DIRENT_SIZE];
    memcpy(bytes, dirent->name, sizeof(dirent->name));
    bytes[11] = dirent->attr;
    bytes[12] = 0;
    bytes[13] = dirent->crt_time_tenth;
    pfatfs__write16(bytes + 16, dirent->crt_time);
    pfatfs__write16(bytes + 18, dirent->crt_date);
    pfatfs__write16(bytes + 20, dirent->fst_clus >> 16);
    pfatfs__write16(bytes + 22, dirent->wrt_time);
    pfatfs__write16(bytes + 24, dirent->wrt_date);
    pfatfs__write16(bytes + 26, dirent->fst_clus & 0xFFFF);
    pfatfs__write32(bytes + 28, dirent->file_size);

    return pfatfs__write(fs, bytes, sizeof(bytes));
}

int pfatfs_mount(pfatfs *fs) {
    int result = pfatfs__parse_bpb(fs);
    if (result != 0)
        return result;

    return 0;
}

static ssize_t pfatfs__get_fat(pfatfs *fs, u32 cluster) {
    u32 fat = PFATFS_FAT_BAD;
    u32 fat_off = fs->fat_offset;
    u8 bytes[sizeof(u32)];
    switch (fs->kind) {
    case PFATFS_FAT12: {
        fat_off += cluster + (cluster >> 1);
        PFATFS_TRY(pfatfs__seek_read(fs, fat_off, bytes, sizeof(u16)));

        u16 packed = pfatfs__read16(bytes);
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
        PFATFS_TRY(pfatfs__seek_read(fs, fat_off, bytes, sizeof(u16)));

        fat = pfatfs__read16(bytes);
        if (fat == PFATFS_FAT16_FREE)
            fat = PFATFS_FAT_FREE;
        else if (fat == PFATFS_FAT16_BAD)
            fat = PFATFS_FAT_BAD;
        else if (fat >= PFATFS_FAT16_EOF)
            fat = PFATFS_FAT_EOF;
        break;
    case PFATFS_FAT32:
        fat_off += cluster * sizeof(u32);
        PFATFS_TRY(pfatfs__seek_read(fs, fat_off, bytes, sizeof(u32)));
        fat = pfatfs__read32(bytes);
        break;
    }

    return fat;
}

static int pfatfs__set_fat(pfatfs *fs, u32 cluster, u32 fat) {
    int result = 0;
    u8 bytes[sizeof(u32)];
    u32 fat_off = fs->fat_offset;

    switch (fs->kind) {
    case PFATFS_FAT12: {
        fat_off += cluster + (cluster >> 1);
        PFATFS_TRY(pfatfs__seek_read(fs, fat_off, bytes, sizeof(u16)));

        if (fat == PFATFS_FAT_FREE)
            fat = PFATFS_FAT12_FREE;
        else if (fat == PFATFS_FAT_BAD)
            fat = PFATFS_FAT12_BAD;
        else if (fat >= PFATFS_FAT_EOF)
            fat = PFATFS_FAT12_EOF;

        u16 old_packed = pfatfs__read16(bytes);
        if ((cluster & 1) != 0)
            fat = (old_packed & 0x000F) | (fat << 4);
        else
            fat = (old_packed & 0xF000) | fat;
        result = pfatfs__seek_write(fs, fat_off, pfatfs__write16(bytes, fat),
                                    sizeof(u16));
    } break;
    case PFATFS_FAT16:
        fat_off += fs->fat_offset + cluster * sizeof(u16);

        if (fat == PFATFS_FAT_FREE)
            fat = PFATFS_FAT16_FREE;
        else if (fat == PFATFS_FAT_BAD)
            fat = PFATFS_FAT16_BAD;
        else if (fat >= PFATFS_FAT_EOF)
            fat = PFATFS_FAT16_EOF;

        result = pfatfs__seek_write(fs, fat_off, pfatfs__write16(bytes, fat),
                                    sizeof(u16));
        break;
    case PFATFS_FAT32:
        fat_off += cluster * sizeof(u32);
        result = pfatfs__seek_write(fs, fat_off, pfatfs__write32(bytes, fat),
                                    sizeof(u32));
        break;
    }

    return result;
}

static ssize_t pfatfs__find_free_cluster(pfatfs *fs, u32 start_cluster,
                                         u32 end_cluster) {
    for (size_t cluster = start_cluster; cluster < end_cluster; ++cluster) {
        u32 fat = pfatfs__get_fat(fs, cluster);
        if (fat == PFATFS_FAT_FREE)
            return cluster;
    }

    return PFATFS_ENOSPC;
}

static ssize_t pfatfs__allocate_cluster(pfatfs *fs) {
    ssize_t cluster = pfatfs__find_free_cluster(fs, 0, fs->cluster_count);
    if (cluster < 0)
        return cluster;

    int result = pfatfs__set_fat(fs, cluster, PFATFS_FAT_EOF);
    if (result < 0)
        return result;

    return cluster;
}

static int pfatfs__read_dirent(pfatfs *fs, u32 loc, pfatfs__dirent *dirent) {
    int result = pfatfs__seek(fs, loc, PFATFS_SEEK_SET);
    if (result == 0)
        result = pfatfs__read_dirent_(fs, dirent);

    return result;
}

static int pfatfs__write_dirent(pfatfs *fs, u32 loc,
                                const pfatfs__dirent *dirent) {
    int result = pfatfs__seek(fs, loc, PFATFS_SEEK_SET);
    if (result == 0)
        result = pfatfs__write_dirent_(fs, dirent);

    return result;
}

static pfatfs_file pfatfs__get_rootdir(pfatfs *fs) {
    u32 rootdir_clus = fs->rootdir.cluster;
    pfatfs_file file = {.type = PFATFS_FILE_DIR,
                        .start_cluster = rootdir_clus,
                        .cluster = rootdir_clus,
                        .dirent_loc = PFATFS_ROOTDIR};
    return file;
}

static ssize_t pfatfs__find_dirent(pfatfs *fs, u32 start, u32 end,
                                   pfatfs__dirent *dirent) {
    u32 offset = start;
    PFATFS_ASSERT(offset % PFATFS_DIRENT_SIZE == 0);
    for (; offset < end; offset += PFATFS_DIRENT_SIZE) {
        pfatfs__dirent temp;
        PFATFS_TRY(pfatfs__read_dirent_(fs, &temp));

        int name0 = temp.name[0];
        if (name0 == 0xe5)
            continue;
        if (name0 == 0x00)
            return PFATFS_ENOENT;

        *dirent = temp;
        break;
    }

    return offset;
}

static int pfatfs__iter_dir_next(pfatfs *fs, pfatfs_file *dir,
                                 pfatfs__dirent *dirent) {
    if (PFATFS_IS_LEGACY_ROOTDIR(fs, dir)) {
        u32 rootdir_offset = fs->rootdir.legacy.offset;
        u32 rootdir_size = fs->rootdir.legacy.size;
        u32 offset = dir->offset;
        PFATFS_TRY(pfatfs__seek(fs, rootdir_offset + offset, PFATFS_SEEK_SET));
        ssize_t new_offset =
            pfatfs__find_dirent(fs, offset, rootdir_size, dirent);
        if (new_offset < 0)
            return new_offset;

        offset = (u32)new_offset;
        PFATFS_ASSERT(offset <= rootdir_size);
        if (offset == rootdir_size)
            return PFATFS_ENOENT;

        dir->offset = offset + PFATFS_DIRENT_SIZE;
        return rootdir_offset + offset;
    } else {
        u32 cluster = dir->cluster;
        u16 cluster_offset = dir->cluster_offset;
        PFATFS_ASSERT(cluster_offset % PFATFS_DIRENT_SIZE == 0);
    retry:
        PFATFS_TRY(pfatfs__seek(
            fs, pfatfs__cluster_to_bytes(fs, cluster, cluster_offset),
            PFATFS_SEEK_SET));
        ssize_t new_offset = pfatfs__find_dirent(fs, cluster_offset,
                                                 fs->bytes_per_cluster, dirent);
        if (new_offset < 0)
            return new_offset;
        cluster_offset = new_offset;

        if (cluster_offset == fs->bytes_per_cluster) {
            ssize_t new_cluster = pfatfs__get_fat(fs, cluster);
            if (new_cluster < 0)
                return new_cluster;

            if (new_cluster == PFATFS_FAT_EOF)
                return PFATFS_EINVAL;
            if (!PFATFS_IS_FAT_REGULAR(new_cluster))
                return PFATFS_ECORRUPTED;

            cluster = new_cluster;
            cluster_offset = 0;
            goto retry;
        }

        dir->cluster = cluster;
        dir->cluster_offset = cluster_offset + PFATFS_DIRENT_SIZE;
        return pfatfs__cluster_to_bytes(fs, cluster, cluster_offset);
    }

    return 0;
}

static ssize_t pfatfs__find_empty_dirent_(pfatfs *fs, u32 start, u32 end) {
    u32 offset = start;
    PFATFS_ASSERT(offset % PFATFS_DIRENT_SIZE == 0);
    for (; offset < end; offset += PFATFS_DIRENT_SIZE) {
        pfatfs__dirent temp;
        PFATFS_TRY(pfatfs__read_dirent_(fs, &temp));
        int name0 = temp.name[0];
        if (name0 == 0xe5 || name0 == 0x00)
            return offset;
    }

    return PFATFS_ENOENT;
}

static ssize_t pfatfs__find_empty_dirent(pfatfs *fs, pfatfs_file *dir) {
    if (PFATFS_IS_LEGACY_ROOTDIR(fs, dir)) {
        u32 rootdir_offset = fs->rootdir.legacy.offset;
        u32 rootdir_size = fs->rootdir.legacy.size;
        u32 offset = dir->offset;
        PFATFS_TRY(pfatfs__seek(fs, rootdir_offset + offset, PFATFS_SEEK_SET));
        ssize_t new_offset =
            pfatfs__find_empty_dirent_(fs, offset, rootdir_size);
        if (new_offset < 0)
            return new_offset;

        offset = (u32)new_offset;
        PFATFS_ASSERT(offset <= rootdir_size);
        if (offset == rootdir_size)
            return PFATFS_ENOENT;

        dir->offset = offset + PFATFS_DIRENT_SIZE;
        return rootdir_offset + offset;
    } else {
        u32 cluster = dir->cluster;
        u16 cluster_offset = dir->cluster_offset;
        PFATFS_ASSERT(cluster_offset % PFATFS_DIRENT_SIZE == 0);
    retry:
        PFATFS_TRY(pfatfs__seek(
            fs, pfatfs__cluster_to_bytes(fs, cluster, cluster_offset),
            PFATFS_SEEK_SET));
        ssize_t new_offset = pfatfs__find_empty_dirent_(fs, cluster_offset,
                                                        fs->bytes_per_cluster);
        if (new_offset < 0)
            return new_offset;
        cluster_offset = new_offset;

        if (cluster_offset == fs->bytes_per_cluster) {
            ssize_t new_cluster = pfatfs__get_fat(fs, cluster);
            if (new_cluster < 0)
                return new_cluster;

            if (new_cluster == PFATFS_FAT_EOF)
                return PFATFS_EINVAL;
            if (!PFATFS_IS_FAT_REGULAR(new_cluster))
                return PFATFS_ECORRUPTED;

            cluster = new_cluster;
            cluster_offset = 0;
            goto retry;
        }

        dir->cluster = cluster;
        dir->cluster_offset = cluster_offset + PFATFS_DIRENT_SIZE;
        return pfatfs__cluster_to_bytes(fs, cluster, cluster_offset);
    }
}

static ssize_t pfatfs__add_dirent(pfatfs *fs, pfatfs_file *dir,
                                  const pfatfs__dirent *dirent) {
    ssize_t dirent_loc = pfatfs__find_empty_dirent(fs, dir);
    if (dirent_loc < 0)
        return dirent_loc;

    PFATFS_TRY(pfatfs__write_dirent(fs, dirent_loc, dirent));
    return 0;
}

static void pfatfs__init_child(pfatfs__dirent *dirent, pfatfs_file *child,
                               u32 dirent_loc) {
    PFATFS_ASSERT(dirent_loc % PFATFS_DIRENT_SIZE == 0);
    memset(child, 0, sizeof(*child));
    memcpy(child->name, dirent->name, sizeof(dirent->name));
    child->type = (dirent->attr & PFATFS_DIRENT_ATTR_DIR) != 0
                      ? PFATFS_FILE_DIR
                      : PFATFS_FILE_REG;
    child->dirent_loc = dirent_loc;
    child->size = dirent->file_size;
    child->cluster = child->start_cluster = dirent->fst_clus;
}

static ssize_t pfatfs__dir_empty(pfatfs *fs, pfatfs_file *dir) {
    PFATFS_ASSERT(dir->type == PFATFS_FILE_DIR);

    for (;;) {
        pfatfs__dirent dirent;
        ssize_t result = pfatfs__iter_dir_next(fs, dir, &dirent);
        if (result == PFATFS_ENOENT)
            break;
        else if (result < 0)
            return result;

        const char *name = dirent.name;
        static const u8 dots[] = "..          ";
        if (memcmp(name, dots, 11) != 0 && memcmp(name, dots + 1, 11) != 0)
            return PFATFS_ENOTEMPTY;
    }

    return 0;
}

static int pfatfs__find_child(pfatfs *fs, pfatfs_file *parent,
                              pfatfs_file *child, const char *name,
                              size_t name_len) {
    PFATFS_ASSERT(parent->type == PFATFS_FILE_DIR);

    for (;;) {
        pfatfs__dirent dirent;
        ssize_t dirent_loc = pfatfs__iter_dir_next(fs, parent, &dirent);
        if (dirent_loc == PFATFS_ENOENT)
            break;
        else if (dirent_loc < 0)
            return dirent_loc;

        if (pfatfs__83_eq_reg(dirent.name, name, name_len)) {
            pfatfs__init_child(&dirent, child, dirent_loc);
            return 0;
        }
    }

    return PFATFS_ENOENT;
}

int pfatfs_readdir(pfatfs *fs, pfatfs_file *dir, pfatfs_file *child) {
    if (dir->type != PFATFS_FILE_DIR)
        return PFATFS_ENOTDIR;

    pfatfs__dirent dirent;
    ssize_t dirent_loc = pfatfs__iter_dir_next(fs, dir, &dirent);
    if (dirent_loc < 0)
        return dirent_loc;

    pfatfs__init_child(&dirent, child, dirent_loc);
    return 0;
}

ssize_t pfatfs_read(pfatfs *fs, pfatfs_file *file, void *buffer, size_t count) {
    if (file->type != PFATFS_FILE_REG)
        return PFATFS_EISDIR;

    char *cursor = (char *)buffer;
    while (count != 0) {
        if (file->cluster_offset >= fs->bytes_per_cluster) {
            ssize_t new_cluster = pfatfs__get_fat(fs, file->cluster);
            if (new_cluster < 0)
                return new_cluster;

            if (new_cluster == PFATFS_FAT_EOF)
                goto end;
            if (!PFATFS_IS_FAT_REGULAR(new_cluster))
                return PFATFS_ECORRUPTED;

            file->cluster = new_cluster;
            file->cluster_offset = 0;
        }

        size_t to_read = count;
        if (file->cluster_offset + to_read > fs->bytes_per_cluster)
            to_read = fs->bytes_per_cluster - file->cluster_offset;
        if (file->offset + to_read > file->size)
            to_read = file->size - file->offset;

        PFATFS_TRY(pfatfs__seek(
            fs,
            pfatfs__cluster_to_bytes(fs, file->cluster, file->cluster_offset),
            PFATFS_SEEK_SET));
        PFATFS_TRY(pfatfs__read(fs, cursor, to_read));

        cursor += to_read;
        count -= to_read;

        file->offset += to_read;
        if (file->offset == file->size)
            goto end;

        file->cluster_offset += to_read;
        PFATFS_ASSERT(file->cluster_offset <= fs->bytes_per_cluster);
    }

end:
    return cursor - (char *)buffer;
}

ssize_t pfatfs_write(pfatfs *fs, pfatfs_file *file, const void *buffer,
                     size_t count) {
    if (file->type != PFATFS_FILE_REG)
        return PFATFS_EISDIR;

    const char *cursor = (const char *)buffer;
    while (count != 0) {
        if (file->cluster_offset >= fs->bytes_per_cluster) {
            ssize_t next_cluster = pfatfs__get_fat(fs, file->cluster);
            if (next_cluster < 0)
                return next_cluster;

            if (next_cluster == PFATFS_FAT_EOF) {
                next_cluster = pfatfs__allocate_cluster(fs);
                if (next_cluster < 0)
                    return next_cluster;

                PFATFS_TRY(pfatfs__set_fat(fs, file->cluster, next_cluster));
            } else if (!PFATFS_IS_FAT_REGULAR(next_cluster)) {
                return PFATFS_ECORRUPTED;
            }

            file->cluster = next_cluster;
            file->cluster_offset = 0;
        }

        size_t to_write = count;
        if (file->cluster_offset + to_write > fs->bytes_per_cluster)
            to_write = fs->bytes_per_cluster - file->cluster_offset;

        PFATFS_TRY(pfatfs__seek(
            fs,
            pfatfs__cluster_to_bytes(fs, file->cluster, file->cluster_offset),
            PFATFS_SEEK_SET));
        PFATFS_TRY(pfatfs__write(fs, cursor, to_write));

        cursor += to_write;
        count -= to_write;

        file->offset += to_write;
        file->cluster_offset += to_write;
        PFATFS_ASSERT(file->cluster_offset <= fs->bytes_per_cluster);
    }

    return cursor - (const char *)buffer;
}

int pfatfs_truncate(pfatfs *fs, pfatfs_file *file, size_t length) {
    if (file->type != PFATFS_FILE_REG)
        return PFATFS_EISDIR;

    // find cluster specified by 'length'
    PFATFS_TRY(pfatfs_seek(fs, file, length, PFATFS_SEEK_SET));
    // iterate linker list of clusters marking them all 'empty'
    ssize_t cluster = pfatfs__get_fat(fs, file->cluster);
    if (cluster < 0)
        return cluster;

    PFATFS_TRY(pfatfs__set_fat(fs, file->cluster, PFATFS_FAT_EOF));
    if (PFATFS_IS_FAT_REGULAR(cluster)) {
        for (;;) {
            // note that if read fails here some clusters are not freed
            ssize_t next_cluster = pfatfs__get_fat(fs, cluster);
            if (next_cluster < 0)
                return next_cluster;

            if (next_cluster == PFATFS_FAT_EOF)
                break;
            if (!PFATFS_IS_FAT_REGULAR(next_cluster))
                return PFATFS_ECORRUPTED;

            cluster = next_cluster;
            PFATFS_TRY(pfatfs__set_fat(fs, cluster, PFATFS_FAT_FREE));
        }
    }

    pfatfs__dirent dirent;
    PFATFS_TRY(pfatfs__read_dirent(fs, file->dirent_loc, &dirent));
    dirent.file_size = length;
    PFATFS_TRY(pfatfs__write_dirent(fs, file->dirent_loc, &dirent));

    return 0;
}

int pfatfs_seek(pfatfs *fs, pfatfs_file *file, ssize_t offset,
                pfatfs_whence whence) {
    u32 new_offset;
    switch (whence) {
    case PFATFS_SEEK_CUR:
        new_offset = file->offset + offset;
        break;
    case PFATFS_SEEK_SET:
        new_offset = offset;
        break;
    case PFATFS_SEEK_END:
        new_offset = file->size + offset;
        break;
    default:
        return PFATFS_EINVAL;
    }

    if (new_offset > file->size)
        return PFATFS_EINVAL;

    if (new_offset == file->offset)
        return 0;

    if (new_offset < file->offset) {
        file->offset = 0;
        file->cluster_offset = 0;
        file->cluster = file->start_cluster;
    }

    PFATFS_ASSERT(new_offset >= file->offset);
    while (file->offset != new_offset) {
        if (file->cluster_offset >= fs->bytes_per_cluster) {
            ssize_t new_cluster = pfatfs__get_fat(fs, file->cluster);
            if (new_cluster < 0)
                return new_cluster;

            if (!PFATFS_IS_FAT_REGULAR(new_cluster))
                return PFATFS_ECORRUPTED;

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

int pfatfs_openv(pfatfs *fs, const char *filename, size_t filename_len,
                 pfatfs_file *file) {
    pfatfs_file search = pfatfs__get_rootdir(fs);
    for (pfatfs__fpath_iter iter = pfatfs__iter_fpath(filename, filename_len);
         !iter.is_finished; pfatfs__iter_fpath_next(&iter)) {

        pfatfs_file child = {0};
        int result =
            pfatfs__find_child(fs, &search, &child, iter.part, iter.part_len);
        if (result != 0)
            return result;

        search = child;
        if (iter.is_last)
            break;
    }

    *file = search;

    return 0;
}

static int pfatfs__init_basename(pfatfs__dirent *dirent, const char *filename,
                                 size_t filename_len) {
    memset(dirent->name, ' ', sizeof(dirent->name));
    const char *before_dot = filename;
    for (size_t cursor = 0;
         before_dot < filename + filename_len && *before_dot != '.';
         ++cursor, ++before_dot) {
        u8 c = pfatfs__toupper(*before_dot);
        if (pfatfs__is_illegal_dirname(c))
            return PFATFS_EINVAL;
        dirent->name[cursor] = c;
    }

    const char *after_dot = before_dot + 1;
    for (size_t cursor = 8; after_dot < filename + filename_len && cursor < 11;
         ++cursor, ++after_dot) {
        u8 c = pfatfs__toupper(*after_dot);
        if (pfatfs__is_illegal_dirname(c))
            return PFATFS_EINVAL;
        dirent->name[cursor] = c;
    }

    if (after_dot < filename + filename_len)
        return PFATFS_EINVAL;

    return 0;
}

static int pfatfs__dirent_create(const char *filename, size_t filename_len,
                                 const pfatfs_file_create_info *info,
                                 pfatfs__dirent *dirent) {
    int result = pfatfs__init_basename(dirent, filename, filename_len);
    if (result >= 0 && info != NULL) {
        dirent->attr = info->attrs;
        dirent->crt_time_tenth = (info->time.secs & 0x1) * 10;
        dirent->crt_time = pfatfs__encode_time(info->time);
        dirent->crt_date = dirent->lst_acc_date = dirent->wrt_date =
            pfatfs__encode_date(info->date);
    }

    return result;
}

int pfatfs_createv(pfatfs *fs, const char *filename, size_t filename_len,
                   const pfatfs_file_create_info *info, pfatfs_file *file) {
    size_t basename_idx = pfatfs__extract_basename(filename, filename_len);
    pfatfs_file dir;
    PFATFS_TRY(pfatfs_openv(fs, filename, basename_idx, &dir));
    PFATFS_ASSERT(dir.type == PFATFS_FILE_DIR);

    ssize_t first_cluster = pfatfs__allocate_cluster(fs);
    if (first_cluster < 0)
        return first_cluster;

    pfatfs__dirent dirent = {0};
    PFATFS_TRY(pfatfs__dirent_create(
        filename + basename_idx, filename_len - basename_idx, info, &dirent));
    dirent.fst_clus = first_cluster;
    ssize_t dirent_loc = pfatfs__add_dirent(fs, &dir, &dirent);
    if (dirent_loc < 0)
        return dirent_loc;

    pfatfs__init_child(&dirent, file, dirent_loc);
    return 0;
}

int pfatfs_renamev(pfatfs *fs, const char *oldpath, size_t oldpath_len,
                   const char *newpath, size_t newpath_length) {
    size_t new_basename_idx = pfatfs__extract_basename(newpath, newpath_length);
    pfatfs_file new_dir;
    PFATFS_TRY(pfatfs_openv(fs, newpath, new_basename_idx, &new_dir));
    PFATFS_ASSERT(new_dir.type == PFATFS_FILE_DIR);

    pfatfs_file old;
    PFATFS_TRY(pfatfs_openv(fs, oldpath, oldpath_len, &old));
    pfatfs__dirent dirent;
    PFATFS_TRY(pfatfs__read_dirent(fs, old.dirent_loc, &dirent));

    PFATFS_TRY(pfatfs__init_basename(&dirent, newpath, newpath_length));
    ssize_t new_dirent_loc = pfatfs__add_dirent(fs, &new_dir, &dirent);
    if (new_dirent_loc < 0)
        return new_dirent_loc;

    dirent.name[0] = 0xe5;
    return pfatfs__write_dirent(fs, old.dirent_loc, &dirent);
}

int pfatfs_mkdirv(pfatfs *fs, const char *filename, size_t filename_len) {
    pfatfs_file_create_info info = {0};
    info.type = PFATFS_FILE_DIR;
    pfatfs_file file;
    return pfatfs_createv(fs, filename, filename_len, &info, &file);
}

int pfatfs_removev(pfatfs *fs, const char *filename, size_t filename_len) {
    pfatfs_file search;
    PFATFS_TRY(pfatfs_openv(fs, filename, filename_len, &search));
    if (search.type == PFATFS_FILE_DIR) {
        if (pfatfs__is_rootdir(&search))
            return PFATFS_EROOTDIR;
        PFATFS_TRY(pfatfs__dir_empty(fs, &search));
    }

    pfatfs__dirent dirent;
    PFATFS_TRY(pfatfs__read_dirent(fs, search.dirent_loc, &dirent));
    dirent.name[0] = 0xe5;
    return pfatfs__write_dirent(fs, search.dirent_loc, &dirent);
}

int pfatfs_open(pfatfs *fs, const char *filename, pfatfs_file *file) {
    return pfatfs_openv(fs, filename, strlen(filename), file);
}

int pfatfs_rename(pfatfs *fs, const char *oldpath, const char *newpath) {
    return pfatfs_renamev(fs, oldpath, strlen(oldpath), newpath,
                          strlen(newpath));
}

int pfatfs_mkdir(pfatfs *fs, const char *path) {
    return pfatfs_mkdirv(fs, path, strlen(path));
}

int pfatfs_remove(pfatfs *fs, const char *path) {
    return pfatfs_removev(fs, path, strlen(path));
}

int pfatfs_create(pfatfs *fs, const char *filename,
                  const pfatfs_file_create_info *info, pfatfs_file *file) {
    return pfatfs_createv(fs, filename, strlen(filename), info, file);
}

