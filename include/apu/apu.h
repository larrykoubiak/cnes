#ifndef APU_H
#define APU_H

#include "apu/apu_bitfields.h"
#include "apu/pulse.h"
#include "apu/triangle.h"
#include "apu/noise.h"
#include "apu/dmc.h"

typedef struct APU {
    // registers
    PulseChannel pulse1;
    PulseChannel pulse2;
    TriangleChannel triangle;
    NoiseChannel noise;
    DMCChannel dmc;
    uint8_t status;
    uint8_t frame_counter;
    // counters
    uint64_t cycle_count;
    uint64_t frame_sequencer_next;
    uint8_t frame_step;
    // audio sampling
    int sample_rate;
    uint64_t sample_rate_step;
    uint64_t sample_timer;
    // ring buffer
    int16_t sample_buffer[APU_SAMPLE_BUFFER_SIZE];
    uint16_t sample_write_idx;
    uint16_t sample_read_idx;
} APU;

void apu_reset(APU* apu);
void apu_init(APU* apu, int sample_rate);
void apu_write(APU* apu, uint16_t addr, uint8_t value);
uint8_t apu_read(APU* apu, uint16_t addr);
void apu_step(APU* apu);
void apu_update_frame_sequencer(APU* apu);
void apu_output_sample(APU* apu);

#endif