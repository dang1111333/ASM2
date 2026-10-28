#ifndef ALERT_THREAD_H
#define ALERT_THREAD_H

typedef struct {
    unsigned long long timestamp_ms;
    double value;
    char type[16];
    char level[16];
} AlertMsg;

void alert_init(void);
void alert_send(const AlertMsg *alert);
void* alert_thread_func(void *arg);

#endif
