#ifndef LOGGER_THREAD_H
#define LOGGER_THREAD_H

#include "circular_buffer.h"

void logger_init(void);
void logger_send(const SensorData *data);
void* logger_thread_func(void *arg);

#endif
