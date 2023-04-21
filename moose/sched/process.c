#include <sched/sched.h>

__used static struct file *get_file(struct process *p, int fd) {
    return fd < ARRAY_SIZE(p->files) ? p->files[fd] : NULL;
}

__used static int allocate_fd(struct process *p) {
    int result = -1;
    for (size_t i = 0; i < ARRAY_SIZE(p->files) && result == -1; ++i) {
        struct file *test = p->files[i];
        if (test == NULL)
            result = i;
    }
    return result;
}

