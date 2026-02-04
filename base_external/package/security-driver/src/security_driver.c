#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/timekeeping.h>
#include "aesd-circular-buffer.h"

#define DEVICE_NAME "security_sensor"

struct security_dev {
    struct aesd_circular_buffer buffer;
    struct mutex lock;
    struct cdev cdev;
};

static int security_major = 0;
static struct security_dev dev;

int security_open(struct inode *inode, struct file *filp) {
    filp->private_data = &dev;
    return 0;
}

int security_release(struct inode *inode, struct file *filp) {
    return 0;
}

ssize_t security_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    struct security_dev *d = filp->private_data;
    struct aesd_buffer_entry *entry;
    size_t entry_offset = 0;
    size_t bytes_to_copy;
    ssize_t retval = 0;

    if (mutex_lock_interruptible(&d->lock)) return -ERESTARTSYS;

    entry = aesd_circular_buffer_find_entry_offset_for_fpos(&d->buffer, *f_pos, &entry_offset);
    if (entry) {
        bytes_to_copy = entry->size - entry_offset;
        if (bytes_to_copy > count) bytes_to_copy = count;
        if (copy_to_user(buf, entry->buffptr + entry_offset, bytes_to_copy)) {
            retval = -EFAULT;
        } else {
            retval = bytes_to_copy;
            *f_pos += bytes_to_copy;
        }
    }
    mutex_unlock(&d->lock);
    return retval;
}

// Запись имитирует срабатывание датчика. Любой ввод = новое событие.
ssize_t security_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct security_dev *d = filp->private_data;
    struct aesd_buffer_entry new_entry;
    char *event_str;
    struct timespec64 ts;
    const char *overwritten = NULL;

    ktime_get_real_ts64(&ts);
    event_str = kmalloc(128, GFP_KERNEL);
    if (!event_str) return -ENOMEM;

    new_entry.size = snprintf(event_str, 128, "[%lld.%ld] SECURITY ALERT: Sensor triggered!
", (long long)ts.tv_sec, ts.tv_nsec);
    new_entry.buffptr = event_str;

    if (mutex_lock_interruptible(&d->lock)) {
        kfree(event_str);
        return -ERESTARTSYS;
    }

    if (d->buffer.full) overwritten = d->buffer.entry[d->buffer.in_offs].buffptr;
    aesd_circular_buffer_add_entry(&d->buffer, &new_entry);
    if (overwritten) kfree(overwritten);

    mutex_unlock(&d->lock);
    return count;
}

struct file_operations security_fops = {
    .owner = THIS_MODULE,
    .read = security_read,
    .write = security_write,
    .open = security_open,
    .release = security_release,
};

static int __init security_init(void) {
    dev_t dev_num;
    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    security_major = MAJOR(dev_num);
    
    mutex_init(&dev.lock);
    aesd_circular_buffer_init(&dev.buffer);
    
    cdev_init(&dev.cdev, &security_fops);
    dev.cdev.owner = THIS_MODULE;
    cdev_add(&dev.cdev, dev_num, 1);
    
    printk(KERN_INFO "Security Sensor Driver initialized
");
    return 0;
}

static void __exit security_exit(void) {
    uint8_t index;
    struct aesd_buffer_entry *entry;
    dev_t dev_num = MKDEV(security_major, 0);

    cdev_del(&dev.cdev);
    AESD_CIRCULAR_BUFFER_FOREACH(entry, &dev.buffer, index) {
        if (entry->buffptr) kfree(entry->buffptr);
    }
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "Security Sensor Driver exited
");
}

module_init(security_init);
module_exit(security_exit);
MODULE_LICENSE("GPL");
