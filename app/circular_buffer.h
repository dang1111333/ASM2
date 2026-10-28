#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <pthread.h>

#define BUFFER_CAPACITY 64

typedef struct {
    unsigned long long timestamp_ms;
    double temperature;
    double humidity;
} SensorData;

typedef struct {
    SensorData data[BUFFER_CAPACITY];
    int head;
    int tail;
    int count;
    int total_produced;
    int total_consumed;
    int overflow_count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} CircularBuffer;

void cb_init(CircularBuffer *cb);
int cb_push(CircularBuffer *cb, const SensorData *item);
int cb_pop(CircularBuffer *cb, SensorData *item);
void cb_destroy(CircularBuffer *cb);

#endif
