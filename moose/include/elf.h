#pragma once

#include <types.h>

#define ELF_HDR_IDENT_SIZE 16

// File header
struct elf64_hdr {
    u8 ident[ELF_HDR_IDENT_SIZE];
    u16 type;
    u16 machine;
    u32 version;
    u64 entry;
    u64 phoff;
    u64 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;
} __attribute__((packed));

static_assert(sizeof(struct elf64_hdr) == 0x40);

// Program header
struct elf64_phdr {
    u32 type;
    u32 flags;
    u64 offset;
    u64 vaddr;
    u64 paddr;
    u64 filesz;
    u64 memsz;
    u64 align;
} __attribute__((packed));

static_assert(sizeof(struct elf64_phdr) == 0x38);

// Section header
struct elf64_shdr {
    u32 name;
    u32 type;
    u64 flags;
    u64 addr;
    u64 offset;
    u64 size;
    u32 link;
    u32 info;
    u64 addralign;
    u64 entsize;
} __attribute__((packed));

static_assert(sizeof(struct elf64_shdr) == 0x40);
