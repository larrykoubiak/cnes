#include "apu/noise.h"

void noise_write(NoiseChannel* nch, int reg_index, uint8_t value) {
    nch->reg.raw[reg_index] = value;
    switch(reg_index) {
        case 0:
            nch->envelope_start_flag = true;
            break;
        case 1:
            break;
        case 2:
            nch->timer_period = nch->reg.period.period;
            break;
        case 3:
            nch->timer_period = nch->reg.period.period;
            nch->length_counter = length_table[nch->reg.period.length_counter];
            break;
    }
}

void noise_step(NoiseChannel* nch) {
    if (nch->timer_counter == 0) {
        nch->timer_counter = nch->timer_period;
    } else {
        nch->timer_counter--;
    }
}
void noise_step_envelope(NoiseChannel* nch) {
    // TO DO
}
void noise_step_length(NoiseChannel* nch) {
    // TO DO
}

int noise_get_output_amplitude(const NoiseChannel* nch) {
    // TO DO
}