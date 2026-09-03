#include "apu/pulse.h"

static const uint8_t duty_table[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 1, 1},
    {0, 0, 0, 0, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 0, 0},
};

static uint8_t pulse_envelope_period(const PulseChannel *pch) {
    return pch->reg[0] & 0x0FU;
}

static uint8_t pulse_sweep_shift(const PulseChannel *pch) {
    return pch->reg[1] & 0x07U;
}

static int32_t pulse_sweep_target(const PulseChannel *pch, bool is_pulse1) {
    uint8_t shift = pulse_sweep_shift(pch);
    int32_t delta = pch->timer_period >> shift;

    if ((pch->reg[1] & 0x08U) != 0) {
        // Pulse 1 uses one's-complement negation; pulse 2 uses two's-complement.
        return (int32_t)pch->timer_period - delta - (is_pulse1 ? 1 : 0);
    }

    return (int32_t)pch->timer_period + delta;
}

static bool pulse_is_muted(const PulseChannel *pch) {
    if (pch->timer_period < 8) {
        return true;
    }

    if ((pch->reg[1] & 0x08U) == 0) {
        return pulse_sweep_target(pch, false) > 0x07FF;
    }

    return false;
}

void pulse_write(PulseChannel *pch, int reg_index, uint8_t value) {
    if (reg_index < 0 || reg_index > 3) {
        return;
    }

    pch->reg[reg_index] = value;

    switch (reg_index) {
        case 0:
            break;

        case 1:
            pch->sweep_reload = true;
            break;

        case 2:
            pch->timer_period = (pch->timer_period & 0x0700U) | value;
            break;

        case 3:
            pch->timer_period = (uint16_t)(
                (pch->timer_period & 0x00FFU) |
                ((uint32_t)(value & 0x07U) << 8));
            if (pch->enabled) {
                pch->length_counter = apu_length_counter_value(value >> 3);
            }
            pch->duty_index = 0;
            pch->envelope_start_flag = true;
            break;
    }
}

void pulse_set_enabled(PulseChannel *pch, bool enabled) {
    pch->enabled = enabled;
    if (!enabled) {
        pch->length_counter = 0;
    }
}

void pulse_step(PulseChannel *pch) {
    if (pch->timer_counter == 0) {
        pch->timer_counter = pch->timer_period;
        pch->duty_index = (pch->duty_index - 1U) & 0x07U;
    } else {
        pch->timer_counter--;
    }
}

void pulse_step_envelope(PulseChannel *pch) {
    uint8_t period = pulse_envelope_period(pch);

    if (pch->envelope_start_flag) {
        pch->envelope_start_flag = false;
        pch->envelope_volume = 15;
        pch->envelope_divider = period;
    } else if (pch->envelope_divider > 0) {
        pch->envelope_divider--;
    } else {
        pch->envelope_divider = period;
        if (pch->envelope_volume > 0) {
            pch->envelope_volume--;
        } else if ((pch->reg[0] & 0x20U) != 0) {
            pch->envelope_volume = 15;
        }
    }
}

void pulse_step_sweep(PulseChannel *pch, bool is_pulse1) {
    bool divider_is_zero = pch->sweep_divider == 0;
    uint8_t shift = pulse_sweep_shift(pch);
    int32_t target = pulse_sweep_target(pch, is_pulse1);

    if (divider_is_zero && (pch->reg[1] & 0x80U) != 0 && shift > 0 &&
        pch->timer_period >= 8 && target >= 0 && target <= 0x07FF) {
        pch->timer_period = (uint16_t)target;
    }

    if (divider_is_zero || pch->sweep_reload) {
        pch->sweep_divider = (pch->reg[1] >> 4) & 0x07U;
        pch->sweep_reload = false;
    } else {
        pch->sweep_divider--;
    }
}

void pulse_step_length(PulseChannel *pch) {
    if ((pch->reg[0] & 0x20U) == 0 && pch->length_counter > 0) {
        pch->length_counter--;
    }
}

uint8_t pulse_get_output_amplitude(const PulseChannel *pch) {
    if (!pch->enabled || pch->length_counter == 0 || pulse_is_muted(pch)) {
        return 0;
    }

    uint8_t duty = (pch->reg[0] >> 6) & 0x03U;
    if (duty_table[duty][pch->duty_index] == 0) {
        return 0;
    }

    if ((pch->reg[0] & 0x10U) != 0) {
        return pch->reg[0] & 0x0FU;
    }

    return pch->envelope_volume;
}
