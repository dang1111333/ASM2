#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <sys/ioctl.h>
#include "sensor_thread.h"
#include "circular_buffer.h"
#include "../driver/sms_sensor.h"

extern int g_running;
extern int g_interval_ms;
extern int g_verbose;
extern CircularBuffer g_buffer;

void* sensor_thread_func(void *arg) {
    (void)arg;

    struct sched_param sp;
    sp.sched_priority = 50;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);

    int fd = open("/dev/sms_sensor", O_RDONLY);
    if (fd >= 0) {
        ioctl(fd, SMS_SET_INTERVAL, &g_interval_ms);
        close(fd);
    }

    struct timespec next_period;
    clock_gettime(CLOCK_MONOTONIC, &next_period);

    char line[128];
    while (g_running) {
        next_period.tv_nsec += g_interval_ms * 1000000L;
        while (next_period.tv_nsec >= 1000000000L) {
            next_period.tv_nsec -= 1000000000L;
            next_period.tv_sec += 1;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_period, NULL);

        if (!g_running) break;

        int dev_fd = open("/dev/sms_sensor", O_RDONLY);
        if (dev_fd >= 0) {
            ssize_t bytes = read(dev_fd, line, sizeof(line) - 1);
            if (bytes > 0) {
                line[bytes] = '\0';
                SensorData data;
                if (sscanf(line, "%llu,%lf,%lf", &data.timestamp_ms, &data.temperature, &data.humidity) == 3) {
                    cb_push(&g_buffer, &data);
                    if (g_verbose) {
                        printf("[SENSOR SAMPLE] Time: %llu ms | Temp: %.1f C | Humid: %.1f %%\n",
                               data.timestamp_ms, data.temperature, data.humidity);
                    }
                }
            }
            close(dev_fd);
        }
    }

    return NULL;
}
