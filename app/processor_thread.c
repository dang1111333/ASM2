#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "processor_thread.h"
#include "circular_buffer.h"
#include "alert_thread.h"
#include "logger_thread.h"

extern int g_running;
extern double g_thresh_temp;
extern double g_thresh_humid;
extern CircularBuffer g_buffer;

void* processor_thread_func(void *arg) {
    (void)arg;
    SensorData item;

    while (g_running) {
        if (cb_pop(&g_buffer, &item) == 0) {
            if (item.temperature > g_thresh_temp) {
                AlertMsg a;
                a.timestamp_ms = item.timestamp_ms;
                a.value = item.temperature;
                strcpy(a.type, "TEMPERATURE");
                strcpy(a.level, item.temperature > (g_thresh_temp + 5.0) ? "CRITICAL" : "WARNING");
                alert_send(&a);
            }

            if (item.humidity > g_thresh_humid) {
                AlertMsg a;
                a.timestamp_ms = item.timestamp_ms;
                a.value = item.humidity;
                strcpy(a.type, "HUMIDITY");
                strcpy(a.level, "WARNING");
                alert_send(&a);
            }

            logger_send(&item);
        }
    }
    return NULL;
}
