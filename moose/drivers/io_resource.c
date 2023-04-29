#include <arch/amd64/virtmem.h>
#include <assert.h>
#include <bitops.h>
#include <drivers/io_resource.h>
#include <mm/kmalloc.h>
#include <param.h>
#include <sched/locks.h>

static LIST_HEAD(port_regions);
static LIST_HEAD(mem_regions);
static rwlock_t regions_lock = INIT_RWLOCK();

static int check_region(u64 base, u64 size, struct list_head *regions) {
    struct io_resource *res;
    list_for_each_entry(res, regions, list) {
        if (base < res->base + res->size && base + size > res->base) {
            return -1;
        }
    }

    return 0;
}

static struct io_resource *alloc_region(u64 base, u64 size) {
    expects(size);
    struct io_resource *res = kmalloc(sizeof(*res));
    if (res == NULL)
        return NULL;

    res->base = base;
    res->size = size;

    return res;
}

static void release_region(struct io_resource *res) {
    list_remove(&res->list);
    kfree(res);
}

struct io_resource *request_port_region(u64 base, u64 size) {
    struct io_resource *res = alloc_region(base, size);
    if (!res) {
        write_unlock(&regions_lock);
        return NULL;
    }

    res->kind = IO_RES_PORT;

    write_lock(&regions_lock);
    list_add(&res->list, &port_regions);
    write_unlock(&regions_lock);
    return res;
}

void release_port_region(struct io_resource *res) {
    write_lock(&regions_lock);
    release_region(res);
    write_unlock(&regions_lock);
}

struct io_resource *request_mem_region(u64 base, u64 size) {
    struct io_resource *res = alloc_region(base, size);
    if (!res) {
        write_unlock(&regions_lock);
        return NULL;
    }

    res->kind = IO_RES_MEM;

    write_lock(&regions_lock);
    list_add(&res->list, &port_regions);
    write_unlock(&regions_lock);

    base = base & ~(PAGE_SIZE - 1);
    size = align_po2(size, PAGE_SIZE);
    if (map_virtual_region(base, MMIO_VIRTUAL_BASE + base,
                           size >> PAGE_SIZE_BITS)) {
        write_lock(&regions_lock);
        release_region(res);
        write_unlock(&regions_lock);
        return NULL;
    }

    return res;
}

void release_mem_region(struct io_resource *res) {
    u64 base = res->base & ~(PAGE_SIZE - 1);
    u64 size = align_po2(res->size, PAGE_SIZE);
    unmap_virtual_region(base, size);

    write_lock(&regions_lock);
    release_region(res);
    write_unlock(&regions_lock);
}

int check_mem_region(u64 base, u64 size) {
    read_lock(&regions_lock);
    int result = check_region(base, size, &mem_regions);
    read_unlock(&regions_lock);
    return result;
}

int check_io_region(u64 base, u64 size) {
    read_lock(&regions_lock);
    int result = check_region(base, size, &port_regions);
    read_unlock(&regions_lock);
    return result;
}
