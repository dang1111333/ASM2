#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/random.h>
#include <linux/timekeeping.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include "sms_sensor.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Le Minh Dang - MSSV");
MODULE_DESCRIPTION("Assignment 2: Sensor Monitoring Kernel Driver");
MODULE_VERSION("1.0");

#define DEVICE_NAME "sms_sensor"
#define CLASS_NAME  "sms_class"
#define DRIVER_MAJOR 242

static int major_number = DRIVER_MAJOR;
static struct class *sms_class = NULL;
static struct device *sms_device = NULL;
static DEFINE_MUTEX(sensor_mutex);

static unsigned long read_count = 0;
static int last_temp = 250;      /* 25.0 do C */
static int last_humid = 600;     /* 60.0 % */
static int min_interval_ms = 100; /* Mac dinh 100ms */
static time64_t start_sec = 0;

static int sms_open(struct inode *inode, struct file *file) {
    return 0;
}

static int sms_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t sms_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    char kbuf[128];
    int str_len;
    struct timespec64 ts;
    u32 rand_val;
    int temp_int, temp_dec;
    int humid_int, humid_dec;
    unsigned long msec_part;

    /* KHÔNG kiểm tra *offset > 0 để mỗi lần gọi read() đều sinh ra mẫu mới */

    mutex_lock(&sensor_mutex);

    ktime_get_real_ts64(&ts);
    msec_part = (unsigned long)(ts.tv_nsec / 1000000L);

    /* Random nhiệt độ: 15.0 - 45.0 độ C */
    rand_val = get_random_u32();
    last_temp = 150 + (rand_val % 301);

    /* Random độ ẩm: 30.0 - 90.0 % */
    rand_val = get_random_u32();
    last_humid = 300 + (rand_val % 601);

    read_count++;

    temp_int = last_temp / 10;
    temp_dec = last_temp % 10;
    humid_int = last_humid / 10;
    humid_dec = last_humid % 10;

    str_len = snprintf(kbuf, sizeof(kbuf), "%lld%03lu,%d.%d,%d.%d\n",
                       (long long)ts.tv_sec, msec_part,
                       temp_int, temp_dec, humid_int, humid_dec);

    if (copy_to_user(buf, kbuf, str_len)) {
        mutex_unlock(&sensor_mutex);
        return -EFAULT;
    }

    /* Đặt lại offset về 0 để các lần đọc tiếp theo không bị EOF */
    *offset = 0;
    mutex_unlock(&sensor_mutex);

    return str_len;
}
static long sms_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int val;
    if (cmd == SMS_SET_INTERVAL) {
        if (copy_from_user(&val, (int __user *)arg, sizeof(int)))
            return -EFAULT;
        if (val < 10)
            return -EINVAL;
        mutex_lock(&sensor_mutex);
        min_interval_ms = val;
        mutex_unlock(&sensor_mutex);
        pr_info("sms_sensor: Updated min_interval to %d ms\n", val);
        return 0;
    }
    return -ENOTTY;
}

static struct file_operations sms_fops = {
    .owner          = THIS_MODULE,
    .open           = sms_open,
    .release        = sms_release,
    .read           = sms_read,
    .unlocked_ioctl = sms_ioctl,
};

static int sms_proc_show(struct seq_file *m, void *v) {
    struct timespec64 ts;
    long uptime_sec;

    ktime_get_real_ts64(&ts);
    uptime_sec = (long)(ts.tv_sec - start_sec);

    mutex_lock(&sensor_mutex);
    seq_printf(m, "=== SMS Sensor Driver Statistics ===\n");
    seq_printf(m, "Read Count         : %lu\n", read_count);
    seq_printf(m, "Last Temperature   : %d.%d C\n", last_temp / 10, last_temp % 10);
    seq_printf(m, "Last Humidity      : %d.%d %%\n", last_humid / 10, last_humid % 10);
    seq_printf(m, "Min Interval       : %d ms\n", min_interval_ms);
    seq_printf(m, "Driver Uptime      : %ld seconds\n", uptime_sec);
    mutex_unlock(&sensor_mutex);
    return 0;
}

static int sms_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, sms_proc_show, NULL);
}

static const struct proc_ops sms_proc_fops = {
    .proc_open    = sms_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init sms_init(void) {
    int ret;
    struct timespec64 ts;

    ktime_get_real_ts64(&ts);
    start_sec = ts.tv_sec;

    ret = register_chrdev(major_number, DEVICE_NAME, &sms_fops);
    if (ret < 0) return ret;

    sms_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(sms_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(sms_class);
    }

    sms_device = device_create(sms_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(sms_device)) {
        class_destroy(sms_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(sms_device);
    }

    proc_create("sms_stats", 0444, NULL, &sms_proc_fops);
    pr_info("sms_sensor: Driver initialized successfully\n");
    return 0;
}

static void __exit sms_exit(void) {
    remove_proc_entry("sms_stats", NULL);
    device_destroy(sms_class, MKDEV(major_number, 0));
    class_destroy(sms_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("sms_sensor: Driver unloaded\n");
}

module_init(sms_init);
module_exit(sms_exit);
