#include <arch/amd64/virtmem.h>
#include <mm/resource.h>
#include <mm/kmalloc.h>
#include <bitops.h>
#include <kstdio.h>
#include <param.h>

LIST_HEAD(port_regions);
LIST_HEAD(mem_regions);

static struct resource *request_region(u64 base, u64 size,
                                       struct list_head *regions) {
    if (size == 0)
        return NULL;

    struct resource *res;
    list_for_each_entry(res, regions, list) {
        if (base < res->base + res->size &&
            base + size > res->base) {
            if (base == res->base && size == res->size)
                return res;

            return NULL;
        }
    }

    struct resource *new_res = kmalloc(sizeof(*new_res));
    if (new_res == NULL)
        return NULL;

    new_res->base = base;
    new_res->size = size;

    list_add(&new_res->list, regions);

    return new_res;
}

static void release_region(struct resource *res) {
    list_remove(&res->list);
    kfree(res);
}

struct resource *request_port_region(u64 base, u64 size) {
    return request_region(base, size, &port_regions);
}

void release_port_region(struct resource *res) {
    release_region(res);
}

struct resource *request_mem_region(u64 base, u64 size) {
    struct resource *res = request_region(base, size, &mem_regions);
    if (res == NULL)
        return NULL;

    base = base & ~(PAGE_SIZE - 1);
    size = align_po2(size, PAGE_SIZE);
    if (map_virtual_region(base, MMIO_VIRTUAL_BASE + base,
                       size >> PAGE_SIZE_BITS)) {
        release_region(res);
        return NULL;
    }

    return res;
}

void release_mem_region(struct resource *res) {
    release_region(res);
    u64 base = res->base & ~(PAGE_SIZE - 1);
    u64 size = align_po2(res->size, PAGE_SIZE);
    unmap_virtual_region(base, size);
}
