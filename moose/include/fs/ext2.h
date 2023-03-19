#pragma once

#include <fs/vfs.h>
#include <types.h>

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
    char name[];
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

int ext2_mount(struct superblock *sb);
