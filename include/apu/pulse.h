#ifndef PULSE_CHANNEL_H
#define PULSE_CHANNEL_H

#include "apu/apu_bitfields.h"

typedef struct {
    union {
        unsigned char raw[4];
        struct {
            Envelope envelope;
            Sweep sweep;
            Timer timer;
        };
    } reg;
    bool enabled;
    uint16_t timer_counter;
    uint16_t timer_period;
    uint8_t duty_index;

    bool envelope_start_flag;
    uint8_t envelope_divider;
    uint8_t enveloper_volume;

    uint8_t sweep_divider;
    uint8_t sweep_reload;

    uint8_t length_counter;
} PulseChannel;

void pulse_write(PulseChannel *pch, int reg_index, uint8_t value);

void pulse_step(PulseChannel *pch);
void pulse_step_envelope(PulseChannel *pch);
void pulse_step_sweep(PulseChannel *pch);
void pulse_step_length(PulseChannel *pch);

int pulse_get_output_amplitude(const PulseChannel *pch);

#endif