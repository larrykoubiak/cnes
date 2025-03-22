#ifndef TRIANGLE_CHANNEL_H
#define TRIANGLE_CHANNEL_H

#include "apu/apu_bitfields.h"

typedef struct {
    union {
        uint8_t raw[4];
        struct {
            LinearCounter counter;
            uint8_t       unused;
            Timer         timer;
        };
    } reg;
    bool     enabled;

    uint16_t timer_counter;
    uint16_t timer_period;

    uint8_t  sequence_index;

    bool     linear_reload_flag;
    uint8_t  linear_counter;

    uint8_t  length_counter;

} TriangleChannel;

void triangle_write(TriangleChannel *tch, int reg_index, uint8_t value);

void triangle_step(TriangleChannel *tch);
void triangle_step_linear(TriangleChannel *tch);
void triangle_step_length(TriangleChannel *tch);

int triangle_get_output_amplitude(const TriangleChannel *tch);

#endif