#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "alert_thread.h"

extern int g_running;
int g_alert_count = 0;
static int alert_pipe[2];

void alert_init(void) {
    if (pipe(alert_pipe) < 0) {
        perror("pipe alert");
    }
    int flags = fcntl(alert_pipe[0], F_GETFL, 0);
    fcntl(alert_pipe[0], F_SETFL, flags | O_NONBLOCK);
}

void alert_send(const AlertMsg *alert) {
    write(alert_pipe[1], alert, sizeof(AlertMsg));
}

void* alert_thread_func(void *arg) {
    (void)arg;
    AlertMsg alert;

    while (g_running) {
        ssize_t bytes = read(alert_pipe[0], &alert, sizeof(AlertMsg));
        if (bytes == sizeof(AlertMsg)) {
            g_alert_count++;
            printf("\033[1;31m[ALERT] %s OVER LIMIT! Time: %llu ms | Value: %.1f | Level: %s\033[0m\n",
                   alert.type, alert.timestamp_ms, alert.value, alert.level);
        } else {
            usleep(15000);
        }
    }

    while (read(alert_pipe[0], &alert, sizeof(AlertMsg)) == sizeof(AlertMsg)) {
        g_alert_count++;
        printf("\033[1;31m[ALERT] %s OVER LIMIT! Time: %llu ms | Value: %.1f | Level: %s\033[0m\n",
               alert.type, alert.timestamp_ms, alert.value, alert.level);
    }

    close(alert_pipe[0]);
    close(alert_pipe[1]);
    return NULL;
}
