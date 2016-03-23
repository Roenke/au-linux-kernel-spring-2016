/*
 * TODO parse command line arguments and call proper
 * VSD_IOCTL_* using C function ioctl (see man ioctl).
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "vsd_ioctl.h"

void usage(const char * exec_path) {
	printf("Wrong command line arguments. \n");
	printf("Usage:\n\t %s <size_get> | <size_set> <SIZE_IN_BYTES>\n", exec_path);
}

int get_query(int vsd) {
    vsd_ioctl_get_size_arg_t arg;

    if (ioctl(vsd, VSD_IOCTL_GET_SIZE, &arg) != 0) {
        perror("ioctl");
        return EXIT_FAILURE;
    }

    printf("Size =  %lu\n.", arg.size);

    return EXIT_SUCCESS;
}

int set_query(int vsd, unsigned long new_size) {
    vsd_ioctl_set_size_arg_t arg = { 
        .size = new_size
    };

    if (ioctl(vsd, VSD_IOCTL_SET_SIZE, &arg) != 0) {
        perror("ioctl");
        return EXIT_FAILURE;
    }

    printf("Complete. New size = %lu\n", new_size);

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    const char * exec_path = argv[0];
    int vsd_h = -1;
    int ret_code = EXIT_SUCCESS;

    unsigned long size_in_bytes = 0;

    if ((argc == 3 && strcmp("size_set", argv[1]) == 0 
        && sscanf(argv[2], "%lu", &size_in_bytes) == 1)
        || (argc == 2 && strcmp("size_get", argv[1]) == 0)) {
        
        int vsd_h = open("/dev/vsd", O_RDONLY);
        
        if (vsd_h == -1) {
            perror("open(/dev/vsd)");
            return EXIT_FAILURE;
        }
        
        ret_code = argc == 2 ? get_query(vsd_h) : set_query(vsd_h, size_in_bytes);
        close(vsd_h);
        return ret_code;
    }

    

    usage(exec_path);
    return EXIT_FAILURE;
}
