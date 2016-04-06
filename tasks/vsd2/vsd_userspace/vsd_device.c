#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "vsd_device.h"
#include "../vsd_driver/vsd_ioctl.h"

const char* VSD_PATH = "/dev/vsd";

static int vsd = -1;

int vsd_init()
{
    vsd = open(VSD_PATH, O_RDWR);
    return vsd > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int vsd_deinit()
{
    return close(vsd);
}

int vsd_get_size(size_t *out_size)
{
    vsd_ioctl_get_size_arg_t arg;

    if (ioctl(vsd, VSD_IOCTL_GET_SIZE, &arg))
        return EXIT_FAILURE;

    *out_size = arg.size;

    return EXIT_SUCCESS;
}

int vsd_set_size(size_t size)
{
    vsd_ioctl_set_size_arg_t arg;
    arg.size = size;

    if (ioctl(vsd, VSD_IOCTL_SET_SIZE, &arg))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

ssize_t vsd_read(char* dst, off_t offset, size_t size)
{
    if (lseek(vsd, offset, SEEK_SET) == -1) {
        return EXIT_FAILURE;
    }

    return read(vsd, dst, size);
}

ssize_t vsd_write(const char* src, off_t offset, size_t size)
{
    if (lseek(vsd, offset, SEEK_SET) == -1) {
        return EXIT_FAILURE;
    }

    return write(vsd, src, size);
}

void* vsd_mmap(size_t offset)
{
    size_t size;
    const size_t page_size = getpagesize();

    if (offset % page_size != 0) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    if (vsd_get_size(&size) == EXIT_FAILURE) {
        return MAP_FAILED;
    }

    return mmap(NULL, size - offset, PROT_READ | PROT_WRITE, MAP_SHARED, vsd, offset);
}

int vsd_munmap(void* addr, size_t offset)
{
    size_t size;
    const size_t page_size = getpagesize();
    if (offset % page_size != 0) {
        errno = EINVAL;
        return EXIT_FAILURE;
    }

    if (vsd_get_size(&size) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return munmap(addr, size - offset);
}
