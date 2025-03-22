#include "apu/pulse.h"

void pulse_write(PulseChannel *pch, int reg_index, uint8_t value) {
    pch->reg.raw[reg_index] = value;
    switch(reg_index) {
        case 0:
            pch->envelope_start_flag = true;
            break;
        case 1:
            break;
        case 2:
            pch->timer_period = pch->reg.timer.timer;
            break;
        case 3:
            pch->timer_period = pch->reg.timer.timer;
            pch->length_counter = length_table[pch->reg.timer.length_counter];
            break;
    }
}

void pulse_step(PulseChannel *pch) {
    if (pch->timer_counter == 0) {
        pch->timer_counter = pch->timer_period;
        pch->duty_index = (pch->duty_index + 1) & 7;
    } else {
        pch->timer_counter--;
    }
}
void pulse_step_envelope(PulseChannel *pch) {
    //TO DO
}
void pulse_step_sweep(PulseChannel *pch) {
    //TO DO
}
void pulse_step_length(PulseChannel *pch) {
    //TO DO
}

int pulse_get_output_amplitude(const PulseChannel *pch) {
    //TO DO
}
