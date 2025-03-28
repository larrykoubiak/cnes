#include "apu/pulse.h"

void pulse_write(PulseChannel *pch, int reg_index, uint8_t value) {
    uint16_t low, high;
    pch->reg.raw[reg_index] = value;
    switch(reg_index) {
        case 0:
            pch->envelope_start_flag = true;
            break;
        case 1:
            break;
        case 2:
            low  = pch->reg.raw[2];
            high = pch->reg.raw[3] & 0x07;
            pch->timer_period = (high << 8) | low;
            break;
        case 3:
            low  = pch->reg.raw[2];
            high = value & 0x07;
            pch->timer_period = (high << 8) | low;
            pulse_enable(pch, pch->enabled);
            break;
    }
}

void pulse_enable(PulseChannel *pch, bool enable) {
    pch->enabled = enable;
    if(enable) {
        uint8_t index = pch->reg.timer.length_counter;
        pch->length_counter = length_table[index];
        pch->duty_index = 0;
        pch->envelope_start_flag = true;
    } else {
        pch->length_counter = 0;
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
    if (pch->envelope_start_flag) {
        pch->envelope_start_flag = false;
        pch->envelope_volume = 15;
        pch->envelope_divider = pch->reg.envelope.volume + 1;
    } else {
        if (pch->envelope_divider > 0) {
            pch->envelope_divider--;
        } else {
            pch->envelope_divider = pch->reg.envelope.volume + 1;
            if (pch->envelope_volume > 0) {
                pch->envelope_volume--;
            } else if (pch->reg.envelope.loop_envelope) {
                pch->envelope_volume = 15;
            }
        }
    }
}

void pulse_step_sweep(PulseChannel *pch, bool is_pulse1) {
    if (pch->reg.sweep.enabled && pch->reg.sweep.shift > 0 && !pch->sweep_mute) {
        if (pch->sweep_reload) {
            pch->sweep_divider = pch->reg.sweep.period;
            pch->sweep_reload = false;
        } else if (pch->sweep_divider > 0) {
            pch->sweep_divider--;
        } else {
            pch->sweep_divider = pch->reg.sweep.period;
            uint16_t delta = pch->timer_period >> pch->reg.sweep.shift;
            uint16_t target;
            if (pch->reg.sweep.negate) {
                if (is_pulse1) {
                    target = pch->timer_period - delta;
                } else {
                    target = pch->timer_period - delta - 1;
                }
            } else {
                target = pch->timer_period + delta;
            }
            if (target >= 0x800 || pch->timer_period < 8) {
                pch->sweep_mute = true;
            } else {
                pch->timer_period = target;
            }
        }
    }
    if (pch->sweep_reload) {
        pch->sweep_reload = false;
        pch->sweep_divider = pch->reg.sweep.period;
    }
}

void pulse_step_length(PulseChannel *pch) {
    if (!pch->reg.envelope.loop_envelope && pch->length_counter > 0) {
        pch->length_counter--;
    }
}

int pulse_get_output_amplitude(const PulseChannel *pch) {
    if (!pch->enabled || pch->length_counter == 0 || pch->sweep_mute || pch->timer_period < 8) {
        return 0;
    }
    uint8_t duty_bit = duty_table[pch->reg.envelope.duty][pch->duty_index];
    if (duty_bit == 0) {
        return 0;
    }
    if (pch->reg.envelope.constant_volume) {
        return pch->reg.envelope.volume;
    } else {
        return pch->envelope_volume;
    }
}
