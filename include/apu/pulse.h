#ifndef PULSE_CHANNEL_H
#define PULSE_CHANNEL_H

#include <stdbool.h>
#include <stdint.h>
#include "apu/apu_bitfields.h"

typedef struct {
    uint8_t reg[4];
    bool enabled;
    uint16_t timer_counter;
    uint16_t timer_period;
    uint8_t duty_index;

    bool envelope_start_flag;
    uint8_t envelope_divider;
    uint8_t envelope_volume;

    uint8_t sweep_divider;
    bool sweep_reload;

    uint8_t length_counter;
} PulseChannel;

void pulse_write(PulseChannel *pch, int reg_index, uint8_t value);
void pulse_set_enabled(PulseChannel *pch, bool enabled);
void pulse_step(PulseChannel *pch);
void pulse_step_envelope(PulseChannel *pch);
void pulse_step_sweep(PulseChannel *pch, bool is_pulse1);
void pulse_step_length(PulseChannel *pch);

uint8_t pulse_get_output_amplitude(const PulseChannel *pch);

#endif
