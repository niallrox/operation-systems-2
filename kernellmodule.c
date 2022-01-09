#include <linux/init.h>
#include <linux/device.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/sched.h>
#include <linux/path.h>
#include <linux/fs.h>
#include <linux/fdtable.h>
#include <linux/namei.h>
#include <linux/pagemap.h>
#include <linux/pci.h>
#include <linux/net.h>
#include <linux/pid.h>
#include "kernellmodule.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NiallRox");
MODULE_DESCRIPTION("ioctl_driver");
MODULE_VERSION("1.0");

static struct class *cls;
char buffer[BUF_LEN] = {NULL};
int len_sock = 0;
int len = 0;


static int device_open(struct inode *inode, struct file *file) {
    pr_info("device_open(%p)\n", file);
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    pr_info("device_release(%p,%p)\n", inode, file);
    return 0;
}

static int ittr_files(const void *v, struct file *file, unsigned n)
{
    struct socket *dev = sock_from_file(file);

    if (dev){
        len_sock += sprintf(buffer + len_sock, "socket: State %s\n", dev->state);
        len_sock += sprintf(buffer + len_sock, "socket: Type: %d\n", dev->type);
        len_sock += sprintf(buffer + len_sock, "socket: Flags: %d\n\n", dev->flags);
        if (len_sock>= 90000) return 0;
    }

    return 0;
}


static long device_ioctl(
        struct file *file,
        unsigned int cmd,
        unsigned long arg) {

    char path_arg[BUF_LEN];
    char __user *tmp = (char __user *) arg;

    struct path path;
    len = 0;

    switch (cmd) {
        case IOCTL_GET_SOCKET:
            copy_from_user(path_arg, tmp, BUF_LEN);
            int pid_value;
            len_sock = 0;

             if(kstrtoint(path_arg, 10, &pid_value) != 0)
                 return -1;

            struct task_struct* ts = get_pid_task(find_get_pid(pid_value), PIDTYPE_PID);
            if(ts == NULL){
                copy_to_user(arg, "Pid is incorrect\n", 17);
                return 0;
            }

            memset(buffer, 0, BUF_LEN);

            task_lock(ts);
            iterate_fd(ts->files, 0, ittr_files, NULL);
            task_unlock(ts);

            if(buffer[0] == NULL) len_sock = sprintf(buffer, "no socks\n");


            copy_to_user(arg, buffer, len_sock);

            break;

        case IOCTL_GET_PCIDEV: {

            static struct pci_dev *dev2;
            char buf2[BUF_LEN];

            while ((dev2 = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev2))) {
                len += sprintf(buf2 + len, "pci_dev: Device_ID %d\n", dev2->devfn);
                len += sprintf(buf2 + len, "pci_dev: Class: %x\n", dev2->class);
                len += sprintf(buf2 + len, "pci_dev: Bus_MSP: %x\n", dev2->bus->max_bus_speed);
                len += sprintf(buf2 + len, "pci_dev: Bus_NUM: %d\n\n", dev2->bus->number);
                len += sprintf(buf2 + len, "pci_dev: Vendor_ID: %hu\n\n", dev2->vendor);
                if (len>= 90000) break;
            }

            copy_to_user(arg, buf2, len);

            break;
        }
        default:
            return -ENOTTY;
    }

    return 0;
}

static struct file_operations fops = {
        .open = device_open,
        .release = device_release,
        .unlocked_ioctl = device_ioctl
};

static int __init

m_init(void) {
    //buf = kmalloc(BUF_LEN, GFP_KERNEL);
    pr_info("INIT MODULE\n");

    int retval = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);

    /* Negative values signify an error */
    if (retval < 0) {
        pr_alert("%s failed with %d\n",
                 "Sorry, registering the character device ", retval);
        return retval;
    }

    cls = class_create(THIS_MODULE, DEVICE_FILE_NAME);
    device_create(cls, NULL, MKDEV(MAJOR_NUM, 0), NULL, DEVICE_FILE_NAME);

    pr_info("Device created on /dev/%s\n", DEVICE_FILE_NAME);

    return 0;
}

static void __exit

m_exit(void) {
    device_destroy(cls, MKDEV(MAJOR_NUM, 0));
    class_destroy(cls);

    /* Unregister the device */
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);

    pr_info("DEINIT MODULE\n");
}

module_init(m_init);
module_exit(m_exit);
