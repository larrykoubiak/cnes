#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include "utils/constants.h"

typedef struct {
    uint8_t sample_buffer[APU_SAMPLE_BUFFER_SIZE];
    uint16_t sample_write_idx;
    uint16_t sample_read_idx;
} ring_buffer;

uint8_t read_rb(ring_buffer *rb);
void write_rb(ring_buffer *rb, uint8_t value);
int ring_buffer_fill(ring_buffer* rb);
#endif