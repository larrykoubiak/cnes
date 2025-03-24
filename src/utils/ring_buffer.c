#include "utils/ring_buffer.h"

uint8_t read_rb(ring_buffer *rb) {
    uint8_t value = 128;
    if(rb->sample_read_idx != rb->sample_write_idx) {
        value = rb->sample_buffer[rb->sample_read_idx];
        rb->sample_read_idx++;
        if(rb->sample_read_idx >= APU_SAMPLE_BUFFER_SIZE) {
            rb->sample_read_idx -= APU_SAMPLE_BUFFER_SIZE;
        }
    }
    return value;
}

void write_rb(ring_buffer *rb, uint8_t value) {
    rb->sample_buffer[rb->sample_write_idx] = value;
    rb->sample_write_idx++;
    if(rb->sample_write_idx >= APU_SAMPLE_BUFFER_SIZE) {
        rb->sample_write_idx -= APU_SAMPLE_BUFFER_SIZE;
    }
}