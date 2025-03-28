#include "apu/triangle.h"

void triangle_write(TriangleChannel *tch, int reg_index, uint8_t value) {
    uint16_t low, high;
    tch->reg.raw[reg_index] = value;
    switch(reg_index) {
        case 0:
            tch->linear_reload_flag = true;
            break;
        case 1:
            break;
        case 2:
            low  = tch->reg.raw[2];
            high = tch->reg.raw[3] & 0x07;
            tch->timer_period = (high << 8) | low;
            break;
        case 3:
            low  = tch->reg.raw[2];
            high = value & 0x07;
            tch->timer_period = (high << 8) | low;
            uint8_t index = (value & 0xF8) >> 3;
            tch->length_counter = length_table[index];
            break;
    }
}

void triangle_step(TriangleChannel *tch) {
    if (tch->timer_counter == 0) {
        tch->timer_counter = tch->timer_period;
        tch->sequence_index = (tch->sequence_index + 1) & 31; // cycle modulo 32
    } else {
        tch->timer_counter--;
    }
}

void triangle_step_linear(TriangleChannel *tch) {
    uint8_t linear_counter_load = tch->reg.raw[0] & 0x7F;
    bool control_flag = (tch->reg.raw[0] & 0x80) != 0;
    if (tch->linear_reload_flag) {
        tch->linear_counter = linear_counter_load;
    } else if (tch->linear_counter > 0) {
        tch->linear_counter--;
    }
    if (!control_flag) {
        tch->linear_reload_flag = false;
    }
}

void triangle_step_length(TriangleChannel *tch) {
    if ((tch->reg.raw[0] & 0x80) == 0 && tch->length_counter > 0) {
        tch->length_counter--;
    }
}

int triangle_get_output_amplitude(const TriangleChannel *tch) {
    if (!tch->enabled || tch->length_counter == 0 || tch->linear_counter == 0 || tch->timer_period < 2) {
        return 0;
    }
    return triangle_table[tch->sequence_index];
}