#include "apu/noise.h"

void noise_write(NoiseChannel* nch, int reg_index, uint8_t value) {
    uint16_t low, high;
    nch->reg.raw[reg_index] = value;
    switch(reg_index) {
        case 0:
            nch->envelope_start_flag = true;
            break;
        case 1:
            break;
        case 2:
            nch->reg.period.loop_noise = (value & 0x80) >> 7;
            uint8_t period_index = (value & 0xF);
            nch->timer_period = noise_table[period_index];
            break;
        case 3:
            uint8_t index = (value & 0xF8) >> 3;
            nch->length_counter = length_table[index];
            break;
    }
}

void noise_step(NoiseChannel* nch) {
    if (nch->timer_counter == 0) {
        nch->timer_counter = nch->timer_period;
        uint8_t feedback;
        if (nch->reg.period.loop_noise) {
            feedback = ((nch->shift_register & 1) ^ ((nch->shift_register >> 6) & 1));
        } else {
            feedback = ((nch->shift_register & 1) ^ ((nch->shift_register >> 1) & 1));
        }
        nch->shift_register >>= 1;
        nch->shift_register |= (feedback << 14);
    } else {
        nch->timer_counter--;
    }
}
void noise_step_envelope(NoiseChannel* nch) {
    if (nch->envelope_start_flag) {
        nch->envelope_start_flag = false;
        nch->envelope_volume = 15;
        nch->envelope_divider = nch->reg.envelope.volume;
    } else {
        if (nch->envelope_divider > 0) {
            nch->envelope_divider--;
        } else {
            nch->envelope_divider = nch->reg.envelope.volume;
            if (nch->envelope_volume > 0) {
                nch->envelope_volume--;
            } else if (nch->reg.envelope.loop_envelope) {
                nch->envelope_volume = 15;
            }
        }
    }
}
void noise_step_length(NoiseChannel* nch) {
    if (!nch->reg.envelope.loop_envelope && nch->length_counter > 0) {
        nch->length_counter--;
    }
}

int noise_get_output_amplitude(const NoiseChannel* nch) {
    if (nch->length_counter == 0) {
        return 0;
    }
    if (nch->timer_period < 8) {
        return 0;
    }
    if (nch->shift_register & 1) {
        return 0;
    } else {
        return (nch->reg.envelope.constant_volume ? 
                  nch->reg.envelope.volume : nch->envelope_volume);
    }
}