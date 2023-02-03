#include <device.h>
#include <errno.h>

off_t llseek(struct device *dev, off_t off, int whence) {
    off_t result = dev->ops.llseek(dev, off, whence);
    int rc = 0;
    if (result < 0) {
        errno = -result;
        rc = -1;
    }

    return rc;
}

ssize_t read(struct device *dev, void *buf, size_t buf_size) {
    ssize_t result = dev->ops.read(dev, buf, buf_size);
    ssize_t rc = result;
    if (result < 0) {
        errno = -result;
        rc = -1;
    }

    return rc;
}

ssize_t write(struct device *dev, const void *buf, size_t buf_size) {
    ssize_t result = dev->ops.write(dev, buf, buf_size);
    ssize_t rc = result;
    if (result < 0) {
        errno = -result;
        rc = -1;
    }

    return rc;
}

int flush(struct device *dev) {
    int rc = 0;
    if (dev->ops.flush)
        rc = dev->ops.flush(dev);

    return rc;
}
