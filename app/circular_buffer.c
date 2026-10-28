#include <stdio.h>
#include <string.h>
#include "circular_buffer.h"

extern int g_running;

void cb_init(CircularBuffer *cb) {
    memset(cb, 0, sizeof(CircularBuffer));
    pthread_mutex_init(&cb->mutex, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

int cb_push(CircularBuffer *cb, const SensorData *item) {
    pthread_mutex_lock(&cb->mutex);
    if (cb->count == BUFFER_CAPACITY) {
        cb->overflow_count++;
        pthread_mutex_unlock(&cb->mutex);
        return -1;
    }
    cb->data[cb->head] = *item;
    cb->head = (cb->head + 1) % BUFFER_CAPACITY;
    cb->count++;
    cb->total_produced++;
    pthread_cond_signal(&cb->not_empty);
    pthread_mutex_unlock(&cb->mutex);
    return 0;
}

int cb_pop(CircularBuffer *cb, SensorData *item) {
    pthread_mutex_lock(&cb->mutex);
    while (cb->count == 0 && g_running) {
        pthread_cond_wait(&cb->not_empty, &cb->mutex);
    }
    if (!g_running && cb->count == 0) {
        pthread_mutex_unlock(&cb->mutex);
        return -1; /* Ket thuc khi nhan lenh shutdown */
    }
    *item = cb->data[cb->tail];
    cb->tail = (cb->tail + 1) % BUFFER_CAPACITY;
    cb->count--;
    cb->total_consumed++;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->mutex);
    return 0;
}

void cb_destroy(CircularBuffer *cb) {
    pthread_mutex_lock(&cb->mutex);
    pthread_cond_broadcast(&cb->not_empty);
    pthread_cond_broadcast(&cb->not_full);
    pthread_mutex_unlock(&cb->mutex);
    pthread_mutex_destroy(&cb->mutex);
    pthread_cond_destroy(&cb->not_empty);
    pthread_cond_destroy(&cb->not_full);
}
