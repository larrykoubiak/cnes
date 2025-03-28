#ifndef APU_H
#define APU_H

#include "apu/apu_bitfields.h"
#include "apu/pulse.h"
#include "apu/triangle.h"
#include "apu/noise.h"
#include "apu/dmc.h"
#include "utils/ring_buffer.h"

static const bool clock_length_sweep_4step[4] = {false, true, false, true};
static const bool clock_length_sweep_5step[5] = {false, true, false, false, true};

static const bool clock_envelope_4step[4] = {true, true, true, true};
static const bool clock_envelope_5step[5] = {true, true, true, false, true};

typedef struct APU {
    // registers
    PulseChannel pulse1;
    PulseChannel pulse2;
    TriangleChannel triangle;
    NoiseChannel noise;
    DMCChannel dmc;

    uint8_t status;
    uint8_t frame_mode;
    bool frame_irq_inhibit;
    // counters
    uint64_t frame_cycle_accumulator;
    uint8_t frame_step;
    bool frame_irq_flag;
    // audio sampling
    uint64_t sample_cycle_accumulator;
    // CPU cycle counter
    uint64_t cycle_count;
    // buffer
    ring_buffer sample_buffer;
} APU;

void apu_reset(APU* apu);
void apu_init(APU* apu, int sample_rate);
void apu_write(APU* apu, uint16_t addr, uint8_t value);
uint8_t apu_read(APU* apu, uint16_t addr);
void apu_step(APU* apu);
void apu_update_frame_sequencer(APU* apu);
void apu_output_sample(APU* apu);

#endif