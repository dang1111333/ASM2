#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "circular_buffer.h"
#include "sensor_thread.h"
#include "processor_thread.h"
#include "alert_thread.h"
#include "logger_thread.h"

int g_running = 1;
int g_interval_ms = 500;
double g_thresh_temp = 40.0;
double g_thresh_humid = 80.0;
int g_duration_sec = 60;
int g_verbose = 0;

CircularBuffer g_buffer;
extern int g_alert_count;

void sig_handler(int sig) {
    (void)sig;
    g_running = 0;
    pthread_cond_broadcast(&g_buffer.not_empty);
    pthread_cond_broadcast(&g_buffer.not_full);
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "i:t:h:d:v")) != -1) {
        switch (opt) {
            case 'i': g_interval_ms = atoi(optarg); break;
            case 't': g_thresh_temp = atof(optarg); break;
            case 'h': g_thresh_humid = atof(optarg); break;
            case 'd': g_duration_sec = atoi(optarg); break;
            case 'v': g_verbose = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-i ms] [-t degC] [-h %%] [-d sec] [-v]\n", argv[0]);
                return 1;
        }
    }

    signal(SIGINT, sig_handler);
    cb_init(&g_buffer);
    alert_init();
    logger_init();

    printf("=== Sensor Monitoring System (SMS) Started ===\n");
    printf("Config: interval=%dms, temp_threshold=%.1f, humid_threshold=%.1f, duration=%ds\n",
           g_interval_ms, g_thresh_temp, g_thresh_humid, g_duration_sec);

    pthread_t t_logger, t_alert, t_processor, t_sensor;

    pthread_create(&t_logger, NULL, logger_thread_func, NULL);
    pthread_create(&t_alert, NULL, alert_thread_func, NULL);
    pthread_create(&t_processor, NULL, processor_thread_func, NULL);
    pthread_create(&t_sensor, NULL, sensor_thread_func, NULL);

    int elapsed = 0;
    while (g_running) {
        sleep(1);
        elapsed++;
        if (g_duration_sec > 0 && elapsed >= g_duration_sec) {
            g_running = 0;
            break;
        }
    }

    printf("\nGraceful shutdown requested, joining threads...\n");
    pthread_cond_broadcast(&g_buffer.not_empty);
    pthread_cond_broadcast(&g_buffer.not_full);

    pthread_join(t_sensor, NULL);
    pthread_join(t_processor, NULL);
    pthread_join(t_alert, NULL);
    pthread_join(t_logger, NULL);

    printf("\n=== SUMMARY REPORT ===\n");
    printf("Total Produced Samples : %d\n", g_buffer.total_produced);
    printf("Total Processed Samples: %d\n", g_buffer.total_consumed);
    printf("Buffer Overflow Count  : %d\n", g_buffer.overflow_count);
    printf("Total Alerts Dispatched: %d\n", g_alert_count);
    printf("Log File Path          : /tmp/sms.log\n");

    cb_destroy(&g_buffer);
    return 0;
}
