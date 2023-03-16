#pragma once

#include <list.h>
#include <types.h>

struct resource {
    u64 base;
    u64 size;
    struct list_head list;
};

struct resource *request_port_region(u64 base, u64 size);
struct resource *request_mem_region(u64 base, u64 size);

void release_port_region(struct resource *res);
void release_mem_region(struct resource *res);
