#ifndef SMS_SENSOR_H
#define SMS_SENSOR_H

#include <linux/ioctl.h>

#define SMS_IOCTL_MAGIC 'S'

/* ioctl lenh: cau hinh chu ky doc toi thieu (ms) */
#define SMS_SET_INTERVAL _IOW(SMS_IOCTL_MAGIC, 1, int)

/* Cau truc du lieu sensor duoc doc tu driver */
struct sensor_raw_data {
    unsigned long long timestamp_ms;
    int temp_raw;    /* Goc nhiet do (chia 10 de ra so thap phan) */
    int humid_raw;   /* Goc do am (chia 10 de ra so thap phan) */
};

#endif
