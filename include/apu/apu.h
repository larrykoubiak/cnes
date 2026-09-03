#ifndef APU_H
#define APU_H

#include <stdbool.h>
#include <stdint.h>

#include "apu/pulse.h"
#include "apu/triangle.h"
#include "apu/noise.h"
#include "apu/dmc.h"
#include "utils/ring_buffer.h"

typedef struct {
    float high_pass_90_coefficient;
    float high_pass_440_coefficient;
    float low_pass_14000_coefficient;

    float high_pass_90_previous_input;
    float high_pass_90_previous_output;
    float high_pass_440_previous_input;
    float high_pass_440_previous_output;
    float low_pass_14000_output;
} APUFilter;

typedef struct APU {
    PulseChannel pulse1;
    PulseChannel pulse2;
    TriangleChannel triangle;
    NoiseChannel noise;
    DMCChannel dmc;

    uint8_t frame_mode;
    uint8_t pending_frame_mode;
    uint8_t frame_reset_delay;
    bool frame_irq_inhibit;
    bool frame_irq_flag;
    uint32_t frame_cycle;

    uint32_t sample_rate;
    uint64_t sample_phase;
    double sample_sum;
    uint32_t sample_sum_count;
    uint64_t output_sample_count;
    APUFilter filter;

    uint64_t cycle_count;
    ring_buffer sample_buffer;
} APU;

void apu_reset(APU* apu);
void apu_init(APU* apu, int sample_rate);
void apu_write(APU* apu, uint16_t addr, uint8_t value);
uint8_t apu_read(APU* apu, uint16_t addr);
void apu_step(APU* apu);
void apu_update_frame_sequencer(APU* apu);
void apu_clock_quarter_frame(APU *apu);
void apu_clock_half_frame(APU *apu);
float apu_mix(const APU *apu);
void apu_output_sample(APU* apu);

#endif
