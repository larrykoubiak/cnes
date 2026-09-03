#ifndef NOISE_CHANNEL_H
#define NOISE_CHANNEL_H

#include <stdbool.h>
#include <stdint.h>
#include "apu/apu_bitfields.h"

typedef struct {
    uint8_t reg[4];
    bool enabled;

    uint16_t timer_counter;
    uint16_t timer_period;

    uint16_t shift_register;

    bool     envelope_start_flag;
    uint8_t  envelope_divider;
    uint8_t  envelope_volume;

    uint8_t  length_counter;
} NoiseChannel;

void noise_reset(NoiseChannel *nch);
void noise_write(NoiseChannel* nch, int reg_index, uint8_t value);
void noise_set_enabled(NoiseChannel *nch, bool enabled);

void noise_step(NoiseChannel* nch);
void noise_step_envelope(NoiseChannel* nch);
void noise_step_length(NoiseChannel* nch);

uint8_t noise_get_output_amplitude(const NoiseChannel* nch);

#endif
