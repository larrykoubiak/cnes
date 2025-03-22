#ifndef NOISE_CHANNEL_H
#define NOISE_CHANNEL_H

#include "apu/apu_bitfields.h"

typedef struct {
    union {
        unsigned char raw[4];
        struct {
            Envelope envelope;
            unsigned char unused;
            NoisePeriod period;
        };
    } reg;
    bool     enabled;

    uint16_t timer_counter;
    uint16_t timer_period;

    uint16_t shift_register;

    bool     envelope_start_flag;
    uint8_t  envelope_divider;
    uint8_t  envelope_volume;

    uint8_t  length_counter;
} NoiseChannel;

void noise_write(NoiseChannel* nch, int reg_index, uint8_t value);

void noise_step(NoiseChannel* nch);
void noise_step_envelope(NoiseChannel* nch);
void noise_step_length(NoiseChannel* nch);

int noise_get_output_amplitude(const NoiseChannel* nch);

#endif