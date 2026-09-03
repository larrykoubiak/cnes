#include <string.h>

#include "apu/noise.h"

static const uint16_t noise_period_table[16] = {
    4, 8, 16, 32, 64, 96, 128, 160,
    202, 254, 380, 508, 762, 1016, 2034, 4068
};

void noise_reset(NoiseChannel *nch) {
    memset(nch, 0, sizeof(*nch));
    nch->shift_register = 1;
    nch->timer_period = (uint16_t)(noise_period_table[0] - 1U);
}

void noise_write(NoiseChannel *nch, int reg_index, uint8_t value) {
    if (reg_index < 0 || reg_index > 3) {
        return;
    }

    nch->reg[reg_index] = value;

    switch (reg_index) {
        case 0:
        case 1:
            break;

        case 2:
            // The table is expressed in CPU cycles. A down-counter that fires
            // on zero therefore reloads with period - 1.
            nch->timer_period = (uint16_t)(noise_period_table[value & 0x0FU] - 1U);
            break;

        case 3:
            if (nch->enabled) {
                nch->length_counter = apu_length_counter_value(value >> 3);
            }
            nch->envelope_start_flag = true;
            break;
    }
}

void noise_set_enabled(NoiseChannel *nch, bool enabled) {
    nch->enabled = enabled;
    if (!enabled) {
        nch->length_counter = 0;
    }
}

void noise_step(NoiseChannel *nch) {
    if (nch->timer_counter == 0) {
        nch->timer_counter = nch->timer_period;
        uint8_t tap = (nch->reg[2] & 0x80U) != 0 ? 6U : 1U;
        uint16_t feedback = (uint16_t)((nch->shift_register & 1) ^
                                      ((nch->shift_register >> tap) & 1));
        nch->shift_register = (uint16_t)((nch->shift_register >> 1) |
                                         ((uint32_t)feedback << 14));
    } else {
        nch->timer_counter--;
    }
}

void noise_step_envelope(NoiseChannel *nch) {
    uint8_t period = nch->reg[0] & 0x0FU;

    if (nch->envelope_start_flag) {
        nch->envelope_start_flag = false;
        nch->envelope_volume = 15;
        nch->envelope_divider = period;
    } else if (nch->envelope_divider > 0) {
        nch->envelope_divider--;
    } else {
        nch->envelope_divider = period;
        if (nch->envelope_volume > 0) {
            nch->envelope_volume--;
        } else if ((nch->reg[0] & 0x20U) != 0) {
            nch->envelope_volume = 15;
        }
    }
}

void noise_step_length(NoiseChannel *nch) {
    if ((nch->reg[0] & 0x20U) == 0 && nch->length_counter > 0) {
        nch->length_counter--;
    }
}

uint8_t noise_get_output_amplitude(const NoiseChannel *nch) {
    if (!nch->enabled || nch->length_counter == 0 ||
        (nch->shift_register & 1U) != 0) {
        return 0;
    }

    if ((nch->reg[0] & 0x10U) != 0) {
        return nch->reg[0] & 0x0FU;
    }

    return nch->envelope_volume;
}
