#include "apu/triangle.h"

static const uint8_t triangle_table[32] = {
    15, 14, 13, 12, 11, 10, 9, 8,
    7, 6, 5, 4, 3, 2, 1, 0,
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15
};

void triangle_write(TriangleChannel *tch, int reg_index, uint8_t value) {
    if (reg_index < 0 || reg_index > 3) {
        return;
    }

    tch->reg[reg_index] = value;

    switch (reg_index) {
        case 0:
        case 1:
            break;

        case 2:
            tch->timer_period = (tch->timer_period & 0x0700U) | value;
            break;

        case 3:
            tch->timer_period = (uint16_t)(
                (tch->timer_period & 0x00FFU) |
                ((uint32_t)(value & 0x07U) << 8));
            if (tch->enabled) {
                tch->length_counter = apu_length_counter_value(value >> 3);
            }
            tch->linear_reload_flag = true;
            break;
    }
}

void triangle_set_enabled(TriangleChannel *tch, bool enabled) {
    tch->enabled = enabled;
    if (!enabled) {
        tch->length_counter = 0;
    }
}

void triangle_step(TriangleChannel *tch) {
    if (tch->timer_counter == 0) {
        tch->timer_counter = tch->timer_period;
        if (tch->length_counter > 0 && tch->linear_counter > 0) {
            tch->sequence_index = (tch->sequence_index + 1U) & 0x1FU;
            tch->output = triangle_table[tch->sequence_index];
        }
    } else {
        tch->timer_counter--;
    }
}

void triangle_step_linear(TriangleChannel *tch) {
    if (tch->linear_reload_flag) {
        tch->linear_counter = tch->reg[0] & 0x7FU;
    } else if (tch->linear_counter > 0) {
        tch->linear_counter--;
    }

    if ((tch->reg[0] & 0x80U) == 0) {
        tch->linear_reload_flag = false;
    }
}

void triangle_step_length(TriangleChannel *tch) {
    if ((tch->reg[0] & 0x80U) == 0 && tch->length_counter > 0) {
        tch->length_counter--;
    }
}

uint8_t triangle_get_output_amplitude(const TriangleChannel *tch) {
    // The triangle DAC holds its last value when either counter stops the
    // sequencer. Forcing it to zero creates a click that real hardware does not.
    return tch->output;
}
