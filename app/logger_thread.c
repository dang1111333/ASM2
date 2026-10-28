#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "logger_thread.h"

extern int g_running;
static int log_pipe[2];

void logger_init(void) {
    if (pipe(log_pipe) < 0) {
        perror("pipe logger");
    }
    int flags = fcntl(log_pipe[0], F_GETFL, 0);
    fcntl(log_pipe[0], F_SETFL, flags | O_NONBLOCK);
}

void logger_send(const SensorData *data) {
    write(log_pipe[1], data, sizeof(SensorData));
}

void* logger_thread_func(void *arg) {
    (void)arg;
    FILE *fp = fopen("/tmp/sms.log", "w");
    if (!fp) {
        perror("fopen /tmp/sms.log");
        return NULL;
    }

    fprintf(fp, "timestamp_ms,temperature,humidity\n");
    fflush(fp);

    SensorData item;
    while (g_running) {
        ssize_t bytes = read(log_pipe[0], &item, sizeof(SensorData));
        if (bytes == sizeof(SensorData)) {
            fprintf(fp, "%llu,%.1f,%.1f\n", item.timestamp_ms, item.temperature, item.humidity);
            fflush(fp);
        } else {
            usleep(20000);
        }
    }

    /* Ghi nốt dữ liệu còn tồn */
    while (read(log_pipe[0], &item, sizeof(SensorData)) == sizeof(SensorData)) {
        fprintf(fp, "%llu,%.1f,%.1f\n", item.timestamp_ms, item.temperature, item.humidity);
    }

    fclose(fp);
    close(log_pipe[0]);
    close(log_pipe[1]);
    return NULL;
}
