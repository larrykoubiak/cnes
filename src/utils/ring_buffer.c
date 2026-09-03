#include "utils/ring_buffer.h"

int16_t read_rb(ring_buffer *rb) {
    int16_t value = 0;
    if (rb->sample_count > 0) {
        value = rb->sample_buffer[rb->sample_read_idx];
        rb->sample_read_idx = (rb->sample_read_idx + 1U) % APU_SAMPLE_BUFFER_SIZE;
        rb->sample_count--;
    }
    return value;
}

bool write_rb(ring_buffer *rb, int16_t value) {
    bool overwritten = false;

    if (rb->sample_count == APU_SAMPLE_BUFFER_SIZE) {
        // Keep the newest audio. Old audio only adds latency once the producer
        // has outrun the playback device.
        rb->sample_read_idx = (rb->sample_read_idx + 1U) % APU_SAMPLE_BUFFER_SIZE;
        rb->sample_count--;
        rb->dropped_samples++;
        overwritten = true;
    }

    rb->sample_buffer[rb->sample_write_idx] = value;
    rb->sample_write_idx = (rb->sample_write_idx + 1U) % APU_SAMPLE_BUFFER_SIZE;
    rb->sample_count++;

    return !overwritten;
}

size_t ring_buffer_fill(const ring_buffer *rb) {
    return rb->sample_count;
}

void ring_buffer_clear(ring_buffer *rb) {
    rb->sample_read_idx = rb->sample_write_idx;
    rb->sample_count = 0;
}
