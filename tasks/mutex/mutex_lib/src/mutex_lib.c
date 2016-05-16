#include <mutex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>
#include <errno.h>

static int mutex_fd = -1;
static const char* MUTEX_DEVICE_PATH = "/dev/mutex";

mutex_err_t mutex_init(mutex_t *m)
{
    mutex_ioctl_lock_create_arg_t arg;

    if (ioctl(mutex_fd, MUTEX_IOCTL_LOCK_CREATE, &arg)) {
        perror("ioctl");
        return MUTEX_INTERNAL_ERR;
    }
    
    m->kid = arg.id;
    shared_spinlock_init(&m->spinlock);
    m->kwaiters_cnt = 0;

    return MUTEX_OK;
}

mutex_err_t mutex_deinit(mutex_t *m)
{
    mutex_ioctl_lock_destroy_arg_t arg = { m->kid };

    if (ioctl(mutex_fd, MUTEX_IOCTL_LOCK_DESTROY, &arg) != 0) {
        perror("ioctl");
        return MUTEX_INTERNAL_ERR;
    }

    return MUTEX_OK;
}

mutex_err_t mutex_lock(mutex_t *m)
{
    mutex_ioctl_lock_wake_arg_t arg = { &m->spinlock, m-> kid };
    mutex_err_t ret = MUTEX_OK;
    if (shared_spin_trylock(&m->spinlock)) {
        return MUTEX_OK;
    }

    __sync_add_and_fetch(&m->kwaiters_cnt, 1);

    if (ioctl(mutex_fd, MUTEX_IOCTL_LOCK_WAIT, &arg) != 0) {
        ret = MUTEX_INTERNAL_ERR;
    }

    __sync_sub_and_fetch(&m->kwaiters_cnt, 1);

    return ret;
}

mutex_err_t mutex_unlock(mutex_t *m)
{ 
    mutex_ioctl_lock_wait_arg_t arg = { &m->spinlock, m->kid };

    if (m->kwaiters_cnt == 0) {
        if (!shared_spin_unlock(&m->spinlock)) {
            return MUTEX_INTERNAL_ERR;
        }

        return MUTEX_OK;
    }

    if (ioctl(mutex_fd, MUTEX_IOCTL_LOCK_WAKE, &arg) != 0) {
        perror("ioctl");
        return MUTEX_INTERNAL_ERR;
    }

    return MUTEX_OK;
}

mutex_err_t mutex_lib_init()
{
    if (mutex_fd != -1) {
        return MUTEX_INTERNAL_ERR;
    }

    mutex_fd = open(MUTEX_DEVICE_PATH, O_RDWR);
    if (mutex_fd == -1) {
        perror("open");
        return MUTEX_INTERNAL_ERR;
    }

    return MUTEX_OK;
}

mutex_err_t mutex_lib_deinit()
{
    if(mutex_fd == -1) {
        return MUTEX_INTERNAL_ERR;
    }

    if (close(mutex_fd) == -1) {
        perror("close");
        return MUTEX_INTERNAL_ERR;
    }

    mutex_fd = -1;
    return MUTEX_OK;
}
