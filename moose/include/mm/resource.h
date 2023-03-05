#pragma once

#include <types.h>
#include <list.h>

struct resource {
    u64 base;
    size_t size;
    struct list_head list;
};

struct resource *request_port_region(u64 base, size_t size);
struct resource *request_mem_region(u64 base, size_t size);

void release_port_region(struct resource *res);
void release_mem_region(struct resource *res);
