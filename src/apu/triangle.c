#include "apu/triangle.h"

void triangle_write(TriangleChannel *tch, int reg_index, uint8_t value) {
    tch->reg.raw[reg_index] = value;
    switch(reg_index) {
        case 0:
            tch->linear_reload_flag = true;
            break;
        case 1:
            break;
        case 2:
            tch->timer_period = tch->reg.timer.timer;
            break;
        case 3:
            tch->timer_period = tch->reg.timer.timer;
            tch->length_counter = length_table[tch->reg.timer.length_counter];
            break;
    }
}

void triangle_step(TriangleChannel *tch) {
    if (tch->timer_counter == 0) {
        tch->timer_counter = tch->timer_period;
    } else {
        tch->timer_counter--;
    }
}
void triangle_step_linear(TriangleChannel *tch) {
    // TO DO
}
void triangle_step_length(TriangleChannel *tch) {
    // TO DO
}

int triangle_get_output_amplitude(const TriangleChannel *tch) {
    // TO DO
}