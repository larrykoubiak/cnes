#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "utils/constants.h"

typedef struct {
    int16_t sample_buffer[APU_SAMPLE_BUFFER_SIZE];
    size_t sample_write_idx;
    size_t sample_read_idx;
    size_t sample_count;
    uint64_t dropped_samples;
} ring_buffer;

int16_t read_rb(ring_buffer *rb);
bool write_rb(ring_buffer *rb, int16_t value);
size_t ring_buffer_fill(const ring_buffer *rb);
void ring_buffer_clear(ring_buffer *rb);

#endif
