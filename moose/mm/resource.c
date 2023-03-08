#include <mm/resource.h>
#include <mm/kmalloc.h>

LIST_HEAD(regions);

struct resource *request_port_region(u64 base, u64 size) {
    if (size == 0)
        return NULL;

    struct resource *res;
    list_for_each_entry(res, &regions, list) {
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

    list_add(&new_res->list, &regions);

    return new_res;
}

void release_port_region(struct resource *res) {
    list_remove(&res->list);
    kfree(res);
}
