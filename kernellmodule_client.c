#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>

#include "kernellmodule.h"



static int print_help()
{
    printf("Usage: ./kmod_client [-s|-p]... PATH\n");
}

int main(int argc, char * argv[])
{	
    if (argc < 2 || argc > 4) {
        return print_help();
    }

    bool print_pcidev = false;
    bool print_socket = false;
    char * path = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-s") == 0) {
            print_socket = true;
        }
        else if (strcmp(argv[i], "-p") == 0) {
            print_pcidev = true;
        }
        else if (!path) {
            path = argv[i];
        }
        else {
            return print_help();
        }
    }

    int fd = open("/dev/" DEVICE_FILE_NAME, O_RDONLY);

    if (fd == -1) {
        printf("Cannot open /dev/" DEVICE_FILE_NAME " device, please check your permissions");
        return -1;
    }

    if (print_socket) {
        char buf[BUF_LEN];
        strcpy(buf, path);

        if (ioctl(fd, IOCTL_GET_SOCKET, buf) != 0) {
            printf("Cannot get struct socket for path %s\n", path);
            return -1;
        }

        printf("%s", buf);
    }

    if (print_pcidev) {
        char buf[BUF_LEN];

        if (ioctl(fd, IOCTL_GET_PCIDEV, buf) != 0) {
            printf("Cannot get struct pci_dev for path %s\n", path);
            return -1;
        }

        printf("%s", buf);
    }

    return 0;
}
