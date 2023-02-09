#pragma once

#include <types.h>

// superblock state
#define EXT2_VALID_FS 1
#define EXT2_ERROR_FS 2

// superblock errors
#define EXT2_ERRORS_CONTINUE 1 /* continue as nothing handled */
#define EXT2_ERRORS_RO 2       /* remount read-only */
#define EXT2_ERRORS_PANIC 3    /* cause kernel panic */

// superblock os
#define EXT2_OS_MOOSE 100

#define EXT2_SUPER_MAGIC 0xEF53

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
    u32 block[15];
    u32 i_generation;
    u32 i_file_acl;
    u32 i_dir_acl;
    u32 i_faddr;
    u32 i_osd2[3];
};

static_assert(sizeof(struct ext2_inode) == 128);

struct ext2_fs {
};

int ext2_mount(struct ext2_fs *fs);

