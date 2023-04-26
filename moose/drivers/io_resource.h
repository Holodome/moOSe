#pragma once

#include <moose/list.h>

enum io_resource_kind {
    IO_RES_PORT,
    IO_RES_MEM
};

struct io_resource {
    u64 base;
    u64 size;
    struct list_head list;
    enum io_resource_kind kind;
};

int check_mem_region(u64 base, u64 size);
struct io_resource *request_mem_region(u64 base, u64 size);
void release_mem_region(struct io_resource *res);

int check_port_region(u64 base, u64 size);
struct io_resource *request_port_region(u64 base, u64 size);
void release_port_region(struct io_resource *res);
