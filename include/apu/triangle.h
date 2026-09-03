#ifndef TRIANGLE_CHANNEL_H
#define TRIANGLE_CHANNEL_H

#include <stdbool.h>
#include <stdint.h>
#include "apu/apu_bitfields.h"

typedef struct {
    uint8_t reg[4];
    bool enabled;

    uint16_t timer_counter;
    uint16_t timer_period;

    uint8_t sequence_index;
    uint8_t output;

    bool linear_reload_flag;
    uint8_t linear_counter;

    uint8_t length_counter;

} TriangleChannel;

void triangle_write(TriangleChannel *tch, int reg_index, uint8_t value);
void triangle_set_enabled(TriangleChannel *tch, bool enabled);

void triangle_step(TriangleChannel *tch);
void triangle_step_linear(TriangleChannel *tch);
void triangle_step_length(TriangleChannel *tch);

uint8_t triangle_get_output_amplitude(const TriangleChannel *tch);

#endif
